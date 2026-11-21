/*
 * term.h — a camada de terminal do motor "Make in C".
 *
 * Este é o único módulo que conversa diretamente com o sistema
 * operacional. Todo o resto do motor (e dos jogos) usa apenas as
 * funções declaradas aqui, e por isso funciona igual no Windows,
 * no Linux e no macOS.
 */
#ifndef TERM_H
#define TERM_H

#include <stddef.h>

/* Códigos de tecla devolvidos por term_tecla() e term_esperar_tecla().
 * Teclas comuns chegam com o próprio código ASCII ('a', '1', ' ').
 * Teclas especiais recebem números acima de 255, que nenhum
 * caractere ASCII usa. */
enum {
    TECLA_NENHUMA   = 0,
    TECLA_TAB       = 9,
    TECLA_ENTER     = 10,
    TECLA_ESC       = 27,
    TECLA_BACKSPACE = 127,
    TECLA_CIMA      = 256,
    TECLA_BAIXO,
    TECLA_ESQUERDA,
    TECLA_DIREITA
};

void   term_iniciar(void);        /* prepara o terminal para o jogo        */
void   term_encerrar(void);       /* devolve o terminal ao estado normal   */
int    term_tecla(void);          /* lê uma tecla SEM esperar              */
int    term_esperar_tecla(void);  /* espera até uma tecla ser apertada     */
void   term_escrever(const char *dados, size_t n); /* envia bytes à tela   */
void   term_dormir(int ms);       /* pausa o programa por 'ms' milésimos   */
void   term_esperar_ate(double instante); /* dorme até term_tempo() chegar lá */
double term_tempo(void);          /* segundos desde term_iniciar()         */
void   term_tamanho(int *colunas, int *linhas); /* tamanho da janela       */

#endif
