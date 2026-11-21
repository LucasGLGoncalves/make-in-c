/*
 * tela.c — implementação da tela virtual.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "term.h"
#include "tela.h"

static Celula atual[TELA_A][TELA_L];     /* onde o jogo desenha            */
static Celula anterior[TELA_A][TELA_L];  /* o que o terminal mostra agora   */
static int    redesenhar = 1;            /* 1 = reenviar todas as células   */

#define MAX_GUARDADAS 4
static Celula guardadas[MAX_GUARDADAS][TELA_A][TELA_L];
static int    n_guardadas = 0;

/* ===================== UTF-8 ===================== */
unsigned int utf8_ler(const char **texto)
{
    const unsigned char *s = (const unsigned char *)*texto;
    unsigned int cp;
    int extras, i;

    if (s[0] < 0x80)              { cp = s[0];        extras = 0; }
    else if ((s[0] & 0xE0) == 0xC0) { cp = s[0] & 0x1F; extras = 1; }
    else if ((s[0] & 0xF0) == 0xE0) { cp = s[0] & 0x0F; extras = 2; }
    else if ((s[0] & 0xF8) == 0xF0) { cp = s[0] & 0x07; extras = 3; }
    else { *texto += 1; return '?'; }          /* byte inválido */

    for (i = 1; i <= extras; i++) {
        if ((s[i] & 0xC0) != 0x80) {           /* sequência cortada */
            *texto += i;
            return '?';
        }
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    *texto += 1 + extras;
    return cp;
}

int utf8_escrever(unsigned int cp, char *d)
{
    if (cp < 0x80) {
        d[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        d[0] = (char)(0xC0 | (cp >> 6));
        d[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        d[0] = (char)(0xE0 | (cp >> 12));
        d[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        d[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    d[0] = (char)(0xF0 | (cp >> 18));
    d[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    d[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    d[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

int utf8_largura(const char *texto)
{
    int n = 0;
    while (*texto) {
        utf8_ler(&texto);
        n++;
    }
    return n;
}

/* Copia 'texto' cortando ou completando com espaços até ter exatamente
 * 'colunas' caracteres. (printf("%-20s") conta BYTES, não caracteres,
 * e desalinha qualquer texto com acento.) */
void utf8_ajustar(char *destino, size_t tamanho, const char *texto, int colunas)
{
    size_t usados = 0;
    int n = 0;
    while (*texto != '\0' && n < colunas) {
        const char *antes = texto;
        size_t bytes;
        utf8_ler(&texto);
        bytes = (size_t)(texto - antes);
        if (usados + bytes + 1 > tamanho) break;
        memcpy(destino + usados, antes, bytes);
        usados += bytes;
        n++;
    }
    while (n < colunas && usados + 2 <= tamanho) {
        destino[usados++] = ' ';
        n++;
    }
    destino[usados] = '\0';
}

static const unsigned short CP437[256] = {
    0x0020,0x263A,0x263B,0x2665,0x2666,0x2663,0x2660,0x2022,
    0x25D8,0x25CB,0x25D9,0x2642,0x2640,0x266A,0x266B,0x263C,
    0x25BA,0x25C4,0x2195,0x203C,0x00B6,0x00A7,0x25AC,0x21A8,
    0x2191,0x2193,0x2192,0x2190,0x221F,0x2194,0x25B2,0x25BC,
    0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
    0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
    0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
    0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
    0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
    0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
    0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
    0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
    0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
    0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
    0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
    0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x2302,
    0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,
    0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
    0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,
    0x00FF,0x00D6,0x00DC,0x00A2,0x00A3,0x00A5,0x20A7,0x0192,
    0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,
    0x00BF,0x2310,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
    0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,
    0x2555,0x2563,0x2551,0x2557,0x255D,0x255C,0x255B,0x2510,
    0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x255E,0x255F,
    0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x2567,
    0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256B,
    0x256A,0x2518,0x250C,0x2588,0x2584,0x258C,0x2590,0x2580,
    0x03B1,0x00DF,0x0393,0x03C0,0x03A3,0x03C3,0x00B5,0x03C4,
    0x03A6,0x0398,0x03A9,0x03B4,0x221E,0x03C6,0x03B5,0x2229,
    0x2261,0x00B1,0x2265,0x2264,0x2320,0x2321,0x00F7,0x2248,
    0x00B0,0x2219,0x00B7,0x221A,0x207F,0x00B2,0x25A0,0x00A0
};

unsigned int tela_cp437(int codigo)
{
    return CP437[codigo & 255];
}

/* ===================== DESENHO ===================== */
void tela_limpar(void)
{
    tela_retangulo(0, 0, TELA_L, TELA_A, ' ', CINZA, PRETO);
}

Celula *tela_celula(int x, int y)
{
    if (x < 0 || y < 0 || x >= TELA_L || y >= TELA_A) return NULL;
    return &atual[y][x];
}

void tela_por(int x, int y, unsigned int glifo, int frente, int fundo)
{
    Celula *c = tela_celula(x, y);
    if (c == NULL) return;              /* fora da tela: ignora */
    c->glifo  = glifo;
    c->frente = (unsigned char)(frente & 15);
    c->fundo  = (unsigned char)(fundo & 15);
}

int tela_texto(int x, int y, int frente, int fundo, const char *texto)
{
    int inicio = x;
    while (*texto) {
        unsigned int cp = utf8_ler(&texto);
        tela_por(x, y, cp, frente, fundo);
        x++;
    }
    return x - inicio;
}

int tela_textof(int x, int y, int frente, int fundo, const char *formato, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, formato);
    vsnprintf(buffer, sizeof buffer, formato, args);
    va_end(args);
    return tela_texto(x, y, frente, fundo, buffer);
}

/* Como tela_texto, mas escreve no máximo 'max_colunas' caracteres. */
int tela_texto_max(int x, int y, int frente, int fundo,
                   const char *texto, int max_colunas)
{
    int n = 0;
    while (*texto != '\0' && n < max_colunas) {
        tela_por(x + n, y, utf8_ler(&texto), frente, fundo);
        n++;
    }
    return n;
}

void tela_texto_centro(int y, int frente, int fundo, const char *texto)
{
    int x = (TELA_L - utf8_largura(texto)) / 2;
    tela_texto(x, y, frente, fundo, texto);
}

void tela_retangulo(int x, int y, int l, int a,
                    unsigned int glifo, int frente, int fundo)
{
    int i, j;
    for (j = y; j < y + a; j++)
        for (i = x; i < x + l; i++)
            tela_por(i, j, glifo, frente, fundo);
}

void tela_moldura(int x, int y, int l, int a, int dupla, int frente, int fundo)
{
    unsigned int h  = dupla ? G_H2  : G_H,  v  = dupla ? G_V2  : G_V;
    unsigned int se = dupla ? G_SE2 : G_SE, sd = dupla ? G_SD2 : G_SD;
    unsigned int ie = dupla ? G_IE2 : G_IE, id = dupla ? G_ID2 : G_ID;
    int i;
    for (i = x + 1; i < x + l - 1; i++) {
        tela_por(i, y, h, frente, fundo);
        tela_por(i, y + a - 1, h, frente, fundo);
    }
    for (i = y + 1; i < y + a - 1; i++) {
        tela_por(x, i, v, frente, fundo);
        tela_por(x + l - 1, i, v, frente, fundo);
    }
    tela_por(x, y, se, frente, fundo);
    tela_por(x + l - 1, y, sd, frente, fundo);
    tela_por(x, y + a - 1, ie, frente, fundo);
    tela_por(x + l - 1, y + a - 1, id, frente, fundo);
}

/* ===================== GUARDAR / RECUPERAR ===================== */
void tela_guardar(void)
{
    if (n_guardadas < MAX_GUARDADAS) {
        memcpy(guardadas[n_guardadas], atual, sizeof atual);
        n_guardadas++;
    }
}

void tela_recuperar(void)
{
    if (n_guardadas > 0) {
        n_guardadas--;
        memcpy(atual, guardadas[n_guardadas], sizeof atual);
    }
}

/* ===================== ENVIO AO TERMINAL ===================== */
/* Códigos ANSI de cor de frente, na ordem das cores do DOS.
 * (O ANSI numera as cores em outra ordem: preto, vermelho, verde,
 *  amarelo, azul, magenta, ciano, branco.) Fundo = frente + 10. */
static const int ANSI[16] = {
    30, 34, 32, 36, 31, 35, 33, 37,   /* cores escuras */
    90, 94, 92, 96, 91, 95, 93, 97    /* cores claras  */
};

static char   saida[65536];
static size_t usados = 0;

static void emitir(const char *dados, size_t n)
{
    if (usados + n > sizeof saida) {          /* buffer cheio: esvazia */
        term_escrever(saida, usados);
        usados = 0;
    }
    memcpy(saida + usados, dados, n);
    usados += n;
}

void tela_redesenhar_tudo(void)
{
    redesenhar = 1;
}

void tela_mostrar(void)
{
    int x, y;
    int cursor_x = -1, cursor_y = -1;     /* onde o cursor do terminal está */
    int cor_frente = -1, cor_fundo = -1;  /* cores ativas no terminal       */
    char seq[32];
    int  n;

    if (redesenhar) emitir("\x1b[0m\x1b[2J", 8);

    for (y = 0; y < TELA_A; y++) {
        for (x = 0; x < TELA_L; x++) {
            Celula *c = &atual[y][x];
            Celula *a = &anterior[y][x];
            unsigned int glifo = c->glifo;

            if (!redesenhar && c->glifo == a->glifo &&
                c->frente == a->frente && c->fundo == a->fundo)
                continue;                          /* nada mudou aqui */

            if (x != cursor_x || y != cursor_y) {  /* move o cursor */
                n = sprintf(seq, "\x1b[%d;%dH", y + 1, x + 1);
                emitir(seq, (size_t)n);
            }
            if (c->frente != cor_frente || c->fundo != cor_fundo) {
                n = sprintf(seq, "\x1b[%d;%dm",
                            ANSI[c->frente], ANSI[c->fundo] + 10);
                emitir(seq, (size_t)n);
                cor_frente = c->frente;
                cor_fundo  = c->fundo;
            }
            /* caracteres de controle nunca vão para o terminal */
            if (glifo < 32 || (glifo >= 127 && glifo < 160)) glifo = ' ';
            n = utf8_escrever(glifo, seq);
            emitir(seq, (size_t)n);

            *a = *c;
            cursor_x = x + 1;
            cursor_y = y;
        }
    }
    redesenhar = 0;
    if (usados > 0) {
        term_escrever(saida, usados);
        usados = 0;
    }
}
