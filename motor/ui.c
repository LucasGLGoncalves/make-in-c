/*
 * ui.c — implementação das peças de interface.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"

void ui_janela(int x, int y, int l, int a, const char *titulo, int cor)
{
    tela_retangulo(x, y, l, a, ' ', CINZA, PRETO);
    tela_moldura(x, y, l, a, 1, cor, PRETO);
    if (titulo != NULL && titulo[0] != '\0') {
        int largura = utf8_largura(titulo) + 2;
        int tx = x + (l - largura) / 2;
        tela_textof(tx, y, BRANCO, PRETO, " %s ", titulo);
    }
}

void ui_barra(int x, int y, int largura, int valor, int maximo,
              int cor_cheia, int cor_vazia)
{
    int cheias, i;
    if (maximo <= 0) maximo = 1;
    if (valor < 0) valor = 0;
    if (valor > maximo) valor = maximo;
    cheias = valor * largura / maximo;
    if (valor > 0 && cheias == 0) cheias = 1;  /* um fio de vida aparece */
    for (i = 0; i < largura; i++) {
        if (i < cheias) tela_por(x + i, y, G_BLOCO, cor_cheia, PRETO);
        else            tela_por(x + i, y, G_SOMBRA_LEVE, cor_vazia, PRETO);
    }
}

int ui_quebrar(const char *texto, int largura,
               char linhas[][UI_LINHA], int max_linhas)
{
    int n = 0;
    const char *p = texto;

    while (*p != '\0' && n < max_linhas) {
        const char *inicio = p;
        const char *q = p;
        const char *ultimo_espaco = NULL;
        const char *fim;
        int colunas = 0;
        size_t bytes;

        /* anda até encher a linha, achar '\n' ou o fim do texto */
        while (*q != '\0' && *q != '\n' && colunas < largura) {
            if (*q == ' ') ultimo_espaco = q;
            utf8_ler(&q);
            colunas++;
        }

        if (*q == '\0' || *q == '\n' || *q == ' ')
            fim = q;                 /* coube inteira ou parou num espaço */
        else if (ultimo_espaco != NULL)
            fim = ultimo_espaco;     /* volta até o último espaço         */
        else
            fim = q;                 /* palavra gigante: corta no meio    */

        bytes = (size_t)(fim - inicio);
        if (bytes > UI_LINHA - 1) bytes = UI_LINHA - 1;
        memcpy(linhas[n], inicio, bytes);
        linhas[n][bytes] = '\0';
        n++;

        p = fim;
        if (*p == '\n') p++;               /* consome a quebra de linha  */
        else while (*p == ' ') p++;        /* pula espaços no começo     */
    }
    return n;
}

int ui_texto_quebrado(int x, int y, int largura, int max_linhas,
                      int frente, int fundo, const char *texto)
{
    char linhas[40][UI_LINHA];
    int n, i;
    if (max_linhas > 40) max_linhas = 40;
    n = ui_quebrar(texto, largura, linhas, max_linhas);
    for (i = 0; i < n; i++)
        tela_texto(x, y + i, frente, fundo, linhas[i]);
    return n;
}

int ui_menu(const char *titulo, const char *opcoes[], int n, int inicial)
{
    int largura = utf8_largura(titulo) + 6;
    int altura, x, y, i, escolha = inicial, tecla;

    for (i = 0; i < n; i++) {
        int l = utf8_largura(opcoes[i]) + 8;
        if (l > largura) largura = l;
    }
    if (largura > TELA_L) largura = TELA_L;
    altura = n + 4;
    x = (TELA_L - largura) / 2;
    y = (TELA_A - altura) / 2;
    if (escolha < 0 || escolha >= n) escolha = 0;

    tela_guardar();
    for (;;) {
        tela_recuperar();           /* devolve o fundo e o guarda de novo */
        tela_guardar();
        ui_janela(x, y, largura, altura, titulo, AMARELO);
        for (i = 0; i < n; i++) {
            int marcada = (i == escolha);
            int frente  = marcada ? BRANCO : CINZA;
            int fundo   = marcada ? AZUL : PRETO;
            tela_retangulo(x + 1, y + 2 + i, largura - 2, 1, ' ', frente, fundo);
            if (i < 9) tela_textof(x + 2, y + 2 + i, AMARELO, fundo, "%d", i + 1);
            tela_texto(x + 5, y + 2 + i, frente, fundo, opcoes[i]);
        }
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == TECLA_CIMA || tecla == 'w' || tecla == 'W')
            escolha = (escolha + n - 1) % n;
        else if (tecla == TECLA_BAIXO || tecla == 's' || tecla == 'S')
            escolha = (escolha + 1) % n;
        else if (tecla == TECLA_ENTER || tecla == ' ')
            break;
        else if (tecla == TECLA_ESC) {
            escolha = -1;
            break;
        } else if (tecla >= '1' && tecla <= '9' && tecla - '1' < n) {
            escolha = tecla - '1';
            break;
        }
    }
    tela_recuperar();
    tela_mostrar();
    return escolha;
}

/* Desenha uma janela centralizada com um texto quebrado e um rodapé. */
static void janela_de_texto(const char *titulo, const char *texto,
                            const char *rodape, int cor)
{
    char linhas[18][UI_LINHA];
    int largura_texto = 56, n, i, l, a, x, y, maior = 0;

    n = ui_quebrar(texto, largura_texto, linhas, 18);
    for (i = 0; i < n; i++) {
        int w = utf8_largura(linhas[i]);
        if (w > maior) maior = w;
    }
    if (utf8_largura(titulo) + 2 > maior) maior = utf8_largura(titulo) + 2;
    if (utf8_largura(rodape) + 2 > maior) maior = utf8_largura(rodape) + 2;
    l = maior + 4;
    a = n + 4;
    x = (TELA_L - l) / 2;
    y = (TELA_A - a) / 2;
    ui_janela(x, y, l, a, titulo, cor);
    for (i = 0; i < n; i++)
        tela_texto(x + 2, y + 2 + i, CINZA, PRETO, linhas[i]);
    tela_texto(x + l - 2 - utf8_largura(rodape), y + a - 1,
               CINZA_ESCURO, PRETO, rodape);
}

void ui_mensagem(const char *titulo, const char *texto)
{
    tela_guardar();
    janela_de_texto(titulo, texto, "[tecla]", CIANO);
    tela_mostrar();
    term_esperar_tecla();
    tela_recuperar();
    tela_mostrar();
}

int ui_confirmar(const char *pergunta)
{
    int tecla, resposta = -1;
    tela_guardar();
    janela_de_texto("Confirmar", pergunta, "[S]im / [N]ão", AMARELO);
    tela_mostrar();
    while (resposta < 0) {
        tecla = term_esperar_tecla();
        if (tecla == 's' || tecla == 'S' || tecla == TECLA_ENTER) resposta = 1;
        if (tecla == 'n' || tecla == 'N' || tecla == TECLA_ESC)   resposta = 0;
    }
    tela_recuperar();
    tela_mostrar();
    return resposta;
}

int ui_ler_texto(const char *titulo, char *destino, int tamanho)
{
    int n = 0, tecla, l = tamanho + 6, x, y = TELA_A / 2 - 2;
    if (l < 30) l = 30;
    if (l > TELA_L) l = TELA_L;
    x = (TELA_L - l) / 2;
    destino[0] = '\0';
    tela_guardar();
    for (;;) {
        ui_janela(x, y, l, 5, titulo, VERDE);
        tela_retangulo(x + 2, y + 2, l - 4, 1, ' ', BRANCO, AZUL);
        tela_texto(x + 2, y + 2, BRANCO, AZUL, destino);
        tela_por(x + 2 + n, y + 2, '_', AMARELO, AZUL);
        tela_mostrar();
        tecla = term_esperar_tecla();
        if (tecla == TECLA_ENTER && n > 0) break;
        if (tecla == TECLA_ESC) { n = 0; destino[0] = '\0'; break; }
        if (tecla == TECLA_BACKSPACE && n > 0) destino[--n] = '\0';
        else if (tecla >= 32 && tecla < 127 && n < tamanho - 1 && n < l - 6) {
            destino[n++] = (char)tecla;   /* só ASCII simples */
            destino[n] = '\0';
        }
    }
    tela_recuperar();
    tela_mostrar();
    return n > 0;
}

void log_limpar(Log *log)
{
    log->primeira = 0;
    log->quantidade = 0;
}

void log_adicionar(Log *log, int cor, const char *formato, ...)
{
    int i;
    va_list args;
    if (log->quantidade < LOG_MAX) {
        i = (log->primeira + log->quantidade) % LOG_MAX;
        log->quantidade++;
    } else {                       /* cheio: a mais antiga dá lugar à nova */
        i = log->primeira;
        log->primeira = (log->primeira + 1) % LOG_MAX;
    }
    va_start(args, formato);
    vsnprintf(log->texto[i], LOG_TAM, formato, args);
    va_end(args);
    log->cor[i] = (unsigned char)cor;
}

void log_desenhar(const Log *log, int x, int y, int l, int a)
{
    int linha = y + a - 1;          /* desenhamos de baixo para cima */
    int k;
    tela_retangulo(x, y, l, a, ' ', CINZA, PRETO);
    for (k = log->quantidade - 1; k >= 0 && linha >= y; k--) {
        int i = (log->primeira + k) % LOG_MAX;
        char partes[6][UI_LINHA];
        int n = ui_quebrar(log->texto[i], l, partes, 6);
        /* as 3 mensagens mais novas aparecem coloridas; as outras, apagadas */
        int cor = (k >= log->quantidade - 3) ? log->cor[i] : CINZA_ESCURO;
        while (n > 0 && linha >= y) {
            n--;
            tela_texto(x, linha, cor, PRETO, partes[n]);
            linha--;
        }
    }
}
