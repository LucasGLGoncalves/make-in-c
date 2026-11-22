/*
 * term.c — implementação da camada de terminal.
 *
 * O arquivo tem duas metades: uma para Windows e outra para sistemas
 * POSIX (Linux, macOS, WSL). O pré-processador escolhe qual metade
 * será compilada olhando a macro _WIN32, que só existe no Windows.
 */

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    /* Pede ao sistema as funções POSIX (clock_gettime, nanosleep...).
     * Precisa vir ANTES de qualquer #include. */
    #define _POSIX_C_SOURCE 200809L
    #include <termios.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "term.h"

static int iniciado = 0;

/* Sequências ANSI usadas ao entrar e sair do modo de jogo:
 *   ESC[?1049h  usa a "tela alternativa" (o terminal guarda o que havia antes)
 *   ESC[?25l    esconde o cursor
 *   ESC[?7l     desliga a quebra automática de linha
 *   ESC[2J      limpa a tela                                            */
static const char SEQ_ENTRAR[] = "\x1b[?1049h\x1b[?25l\x1b[?7l\x1b[0m\x1b[2J";
static const char SEQ_SAIR[]   = "\x1b[0m\x1b[?7h\x1b[?25h\x1b[?1049l";

static void ao_receber_sinal(int sinal)
{
    term_encerrar();
    signal(sinal, SIG_DFL);
    raise(sinal);
}

#if defined(_WIN32)
/* ===================================================================
 *                             WINDOWS
 * =================================================================== */
static HANDLE        entrada, saida;
static DWORD         modo_entrada_original, modo_saida_original;
static UINT          pagina_original;
static LARGE_INTEGER frequencia, inicio;

void term_escrever(const char *dados, size_t n)
{
    fwrite(dados, 1, n, stdout);
    fflush(stdout);
}

void term_iniciar(void)
{
    DWORD modo;
    if (iniciado) return;
    entrada = GetStdHandle(STD_INPUT_HANDLE);
    saida   = GetStdHandle(STD_OUTPUT_HANDLE);

    /* saída: liga as sequências ANSI e o UTF-8 */
    GetConsoleMode(saida, &modo_saida_original);
    modo = modo_saida_original | ENABLE_PROCESSED_OUTPUT
                               | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(saida, modo);
    pagina_original = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    /* entrada: só o Ctrl+C continua especial. Sem eco, sem esperar o
     * Enter, sem eventos de mouse e sem a "edição rápida", que congela
     * o programa quando alguém clica na janela. */
    GetConsoleMode(entrada, &modo_entrada_original);
    SetConsoleMode(entrada, ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS);
    FlushConsoleInputBuffer(entrada);     /* esquece teclas antigas */

    QueryPerformanceFrequency(&frequencia);
    QueryPerformanceCounter(&inicio);
    iniciado = 1;
    atexit(term_encerrar);
    signal(SIGINT, ao_receber_sinal);
    term_escrever(SEQ_ENTRAR, sizeof SEQ_ENTRAR - 1);
}

void term_encerrar(void)
{
    if (!iniciado) return;
    term_escrever(SEQ_SAIR, sizeof SEQ_SAIR - 1);
    SetConsoleMode(saida, modo_saida_original);
    SetConsoleMode(entrada, modo_entrada_original);
    SetConsoleOutputCP(pagina_original);
    iniciado = 0;
}

/* O console do Windows guarda uma fila de EVENTOS: tecla apertada,
 * tecla solta, mouse, foco, mudança de tamanho... Lemos a fila, um
 * evento por vez, até achar uma tecla apertada. O resto é descartado.
 * Se a fila acabar sem nenhuma, não esperamos: TECLA_NENHUMA. */
int term_tecla(void)
{
    INPUT_RECORD evento;
    DWORD pendentes, lidos;

    while (GetNumberOfConsoleInputEvents(entrada, &pendentes) && pendentes > 0) {
        const KEY_EVENT_RECORD *k = &evento.Event.KeyEvent;
        if (!ReadConsoleInputW(entrada, &evento, 1, &lidos) || lidos == 0)
            break;
        if (evento.EventType != KEY_EVENT || !k->bKeyDown)
            continue;                         /* não é tecla apertada */
        switch (k->wVirtualKeyCode) {         /* teclas pelo "código virtual" */
            case VK_UP:     return TECLA_CIMA;
            case VK_DOWN:   return TECLA_BAIXO;
            case VK_LEFT:   return TECLA_ESQUERDA;
            case VK_RIGHT:  return TECLA_DIREITA;
            case VK_RETURN: return TECLA_ENTER;
            case VK_BACK:   return TECLA_BACKSPACE;
            case VK_ESCAPE: return TECLA_ESC;
        }
        if (k->uChar.UnicodeChar > 0 && k->uChar.UnicodeChar < 256)
            return k->uChar.UnicodeChar;      /* letra, número, espaço... */
        /* Shift, Ctrl, F1 e companhia: não interessam, seguimos lendo */
    }
    return TECLA_NENHUMA;
}

void term_dormir(int ms)
{
    if (ms > 0) Sleep((DWORD)ms);
}

double term_tempo(void)
{
    LARGE_INTEGER agora;
    QueryPerformanceCounter(&agora);
    return (double)(agora.QuadPart - inicio.QuadPart)
         / (double)frequencia.QuadPart;
}

void term_tamanho(int *colunas, int *linhas)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    /* pede o handle aqui mesmo: os jogos medem a janela ANTES de term_iniciar */
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        *colunas = info.srWindow.Right - info.srWindow.Left + 1;
        *linhas  = info.srWindow.Bottom - info.srWindow.Top + 1;
    } else {
        *colunas = 80;
        *linhas  = 25;
    }
}

#else
/* ===================================================================
 *                      POSIX (Linux, macOS, WSL)
 * =================================================================== */
static struct termios   original;
static struct timespec  inicio;

void term_escrever(const char *dados, size_t n)
{
    /* write() pode escrever só uma parte; repetimos até acabar. */
    while (n > 0) {
        ssize_t escritos = write(STDOUT_FILENO, dados, n);
        if (escritos <= 0) return;
        dados += escritos;
        n     -= (size_t)escritos;
    }
}

void term_iniciar(void)
{
    struct termios cru;
    if (iniciado) return;
    tcgetattr(STDIN_FILENO, &original);
    cru = original;
    cru.c_lflag &= ~(tcflag_t)(ICANON | ECHO); /* sem esperar Enter, sem eco */
    cru.c_iflag &= ~(tcflag_t)(IXON | ICRNL);  /* Ctrl+S livre; Enter = \r  */
    cru.c_cc[VMIN]  = 0;     /* read() não espera por nenhum byte...       */
    cru.c_cc[VTIME] = 0;     /* ...e nem por tempo algum                   */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &cru);
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    iniciado = 1;
    atexit(term_encerrar);
    signal(SIGINT, ao_receber_sinal);
    signal(SIGTERM, ao_receber_sinal);
    term_escrever(SEQ_ENTRAR, sizeof SEQ_ENTRAR - 1);
}

void term_encerrar(void)
{
    if (!iniciado) return;
    term_escrever(SEQ_SAIR, sizeof SEQ_SAIR - 1);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
    iniciado = 0;
}

/* Lê um byte da entrada. Se 'espera_ms' > 0, aguarda até esse tempo
 * pela chegada do byte. Devolve -1 se nada chegou. */
static int ler_byte(int espera_ms)
{
    unsigned char c;
    if (espera_ms > 0) {
        fd_set conjunto;
        struct timeval limite;
        FD_ZERO(&conjunto);
        FD_SET(STDIN_FILENO, &conjunto);
        limite.tv_sec  = 0;
        limite.tv_usec = espera_ms * 1000;
        if (select(STDIN_FILENO + 1, &conjunto, NULL, NULL, &limite) <= 0)
            return -1;
    }
    if (read(STDIN_FILENO, &c, 1) == 1) return c;
    return -1;
}

int term_tecla(void)
{
    int c = ler_byte(0);
    if (c < 0) return TECLA_NENHUMA;
    if (c == 27) {                 /* ESC sozinho ou início de sequência? */
        int c2 = ler_byte(25);
        if (c2 < 0) return TECLA_ESC;
        if (c2 == '[' || c2 == 'O') {
            int c3 = ler_byte(25);
            switch (c3) {
                case 'A': return TECLA_CIMA;
                case 'B': return TECLA_BAIXO;
                case 'C': return TECLA_DIREITA;
                case 'D': return TECLA_ESQUERDA;
            }
            /* sequência que não usamos (F1, Home...): descarta o resto */
            while (c3 >= 0 && !((c3 >= 'A' && c3 <= 'Z') ||
                                (c3 >= 'a' && c3 <= 'z') || c3 == '~'))
                c3 = ler_byte(5);
            return TECLA_NENHUMA;
        }
        return TECLA_ESC;
    }
    if (c == '\r' || c == '\n') return TECLA_ENTER;
    if (c == 8)                 return TECLA_BACKSPACE;
    return c;
}

void term_dormir(int ms)
{
    struct timespec t;
    if (ms <= 0) return;
    t.tv_sec  = ms / 1000;
    t.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&t, NULL);
}

double term_tempo(void)
{
    struct timespec agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);
    return (double)(agora.tv_sec - inicio.tv_sec)
         + (double)(agora.tv_nsec - inicio.tv_nsec) / 1e9;
}

void term_tamanho(int *colunas, int *linhas)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        *colunas = w.ws_col;
        *linhas  = w.ws_row;
    } else {
        *colunas = 80;
        *linhas  = 24;
    }
}
#endif

/* ===================================================================
 *                    PARTE COMUM AOS DOIS SISTEMAS
 * =================================================================== */
int term_esperar_tecla(void)
{
    int tecla;
    while ((tecla = term_tecla()) == TECLA_NENHUMA)
        term_dormir(10);
    return tecla;
}

/* Dorme até o relógio passar de 'instante'. O "+ 1" importa: sem ele,
 * uma espera de 0,4 ms vira (int)0 = 0 ms, e o laço fica girando à toa
 * até o instante chegar (veja o capítulo 18). */
void term_esperar_ate(double instante)
{
    double falta = instante - term_tempo();
    if (falta > 0) term_dormir((int)(falta * 1000.0) + 1);
}
