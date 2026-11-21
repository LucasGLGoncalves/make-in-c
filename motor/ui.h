/*
 * ui.h — peças de interface: janelas, barras, menus, mensagens,
 * quebra de texto e o registro (log) de mensagens do jogo.
 */
#ifndef UI_H
#define UI_H

#define UI_LINHA 320   /* bytes máximos de uma linha depois de quebrada */

/* ---- Desenho simples (não esperam tecla) ---- */
void ui_janela(int x, int y, int l, int a, const char *titulo, int cor);
void ui_barra(int x, int y, int largura, int valor, int maximo,
              int cor_cheia, int cor_vazia);
int  ui_quebrar(const char *texto, int largura,
                char linhas[][UI_LINHA], int max_linhas);
int  ui_texto_quebrado(int x, int y, int largura, int max_linhas,
                       int frente, int fundo, const char *texto);

/* ---- Janelas que esperam o jogador (bloqueiam até a resposta) ---- */
int  ui_menu(const char *titulo, const char *opcoes[], int n, int inicial);
void ui_mensagem(const char *titulo, const char *texto);
int  ui_confirmar(const char *pergunta);
int  ui_ler_texto(const char *titulo, char *destino, int tamanho);

/* ---- Registro de mensagens ("log") ---- */
#define LOG_MAX 50
#define LOG_TAM 160
typedef struct {
    char          texto[LOG_MAX][LOG_TAM];
    unsigned char cor[LOG_MAX];
    int           primeira;     /* índice da mensagem mais antiga */
    int           quantidade;   /* quantas mensagens estão guardadas */
} Log;

void log_limpar(Log *log);
void log_adicionar(Log *log, int cor, const char *formato, ...);
void log_desenhar(const Log *log, int x, int y, int l, int a);

#endif
