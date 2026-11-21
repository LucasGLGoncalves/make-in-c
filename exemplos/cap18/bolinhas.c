/* bolinhas.c — um laço de jogo em tempo real, com estados. */
#include <stdio.h>
#include "term.h"
#include "tela.h"
#include "rng.h"

#define N_BOLAS 6
#define TICKS_SEG 20

typedef enum { TITULO, JOGANDO, PAUSADO, FIM } Estado;
typedef struct { int x, y, dx, dy; } Bola;

int main(void)
{
    Bola bolas[N_BOLAS];
    Estado estado = TITULO;
    int x = 40, y = 12, vidas = 3, i, quadros = 0, fps = 0;
    long ticks = 0;
    double proximo, ultimo_segundo;

    term_iniciar();
    rng_semear(rng_semente_do_relogio());
    for (i = 0; i < N_BOLAS; i++) {
        bolas[i].x = rng_intervalo(2, 77);
        bolas[i].y = rng_intervalo(2, 5);
        bolas[i].dx = rng_chance(50) ? 1 : -1;
        bolas[i].dy = 1;
    }
    proximo = ultimo_segundo = term_tempo();

    while (estado != FIM) {
        int tecla;
        /* 1. ENTRADA */
        while ((tecla = term_tecla()) != TECLA_NENHUMA) {
            if (tecla == 'q' || tecla == TECLA_ESC) estado = FIM;
            else if (estado == TITULO) estado = JOGANDO;
            else if (tecla == 'p') estado = (estado == PAUSADO) ? JOGANDO : PAUSADO;
            else if (estado == JOGANDO) {
                if (tecla == TECLA_ESQUERDA && x > 1)  x--;
                if (tecla == TECLA_DIREITA  && x < 78) x++;
                if (tecla == TECLA_CIMA     && y > 1)  y--;
                if (tecla == TECLA_BAIXO    && y < 21) y++;
            }
        }
        /* 2. LÓGICA, num ritmo fixo */
        while (term_tempo() >= proximo) {
            if (estado == JOGANDO && ++ticks % 2 == 0)
                for (i = 0; i < N_BOLAS; i++) {
                    Bola *b = &bolas[i];
                    if (b->x + b->dx < 1 || b->x + b->dx > 78) b->dx = -b->dx;
                    if (b->y + b->dy < 1 || b->y + b->dy > 21) b->dy = -b->dy;
                    b->x += b->dx;
                    b->y += b->dy;
                    if (b->x == x && b->y == y && --vidas == 0) estado = FIM;
                }
            proximo += 1.0 / TICKS_SEG;
        }
        /* 3. DESENHO */
        tela_limpar();
        tela_moldura(0, 0, 80, 23, 0, CINZA_ESCURO, PRETO);
        for (i = 0; i < N_BOLAS; i++)
            tela_por(bolas[i].x, bolas[i].y, G_BOLINHA, VERMELHO_CLARO, PRETO);
        tela_por(x, y, '@', AMARELO, PRETO);
        tela_textof(1, 23, CINZA, PRETO, "Vidas %d   Ticks %ld   %d quadros/s   p pausa  q sai",
                    vidas, ticks, fps);
        if (estado == TITULO) tela_texto_centro(10, BRANCO, AZUL, " Fuja das bolinhas! Aperte uma tecla. ");
        if (estado == PAUSADO) tela_texto_centro(10, PRETO, AMARELO, " PAUSA ");
        tela_mostrar();
        quadros++;
        if (term_tempo() - ultimo_segundo >= 1.0) {      /* mede os quadros/s */
            fps = quadros;
            quadros = 0;
            ultimo_segundo += 1.0;
        }
        /* 4. ESPERA */
        term_esperar_ate(proximo);
    }
    term_encerrar();
    printf("Fim! Você sobreviveu %ld ticks.\n", ticks);
    return 0;
}
