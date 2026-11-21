/*
 * arena.c — quatro tipos de "inteligência": o andarilho, o patrulheiro,
 * o perseguidor e o atirador. (Uma homenagem ao Topdown Walking
 * Adventure, que tinha Wanderers, Patrollers e Shooters.)
 */
#include <stdio.h>
#include <stdlib.h>
#include "term.h"
#include "tela.h"
#include "rng.h"
#include "mapa.h"
#include "fov.h"

#define MAX_ENT 64
enum { CHAO, PAREDE, PORTA, PORTA_TRANCADA, OURO, CHAVE };
enum { ANDARILHO, PATRULHEIRO, PERSEGUIDOR, ATIRADOR, TIRO };

static const TileDef TILES[] = {
    { G_PONTO,        CINZA_ESCURO, PRETO, 0,                  "chão"   },
    { G_BLOCO,        CINZA,        PRETO, T_SOLIDO | T_OPACO, "parede" },
    { G_SOMBRA_MEDIA, MARROM,       PRETO, T_OPACO,            "porta"  },
    { G_SOMBRA_MEDIA, AMARELO,      PRETO, T_SOLIDO | T_OPACO, "porta trancada" },
    { '$',            AMARELO,      PRETO, 0,                  "ouro"   },
    { G_FEMININO,     AMARELO,      PRETO, 0,                  "chave"  }
};
static const char LEGENDA[] = ".#+D$k";

typedef struct { int tipo, x, y, dx, dy, vivo, espera; } Ent;
static Ent ent[MAX_ENT];
static int n_ent = 0, jx, jy;

static void criar(int tipo, int x, int y, int dx, int dy)
{
    int i;
    for (i = 0; i < n_ent && ent[i].vivo; i++) { }   /* reaproveita um morto */
    if (i == n_ent) {
        if (n_ent >= MAX_ENT) return;
        n_ent++;
    }
    ent[i].tipo = tipo; ent[i].x = x; ent[i].y = y;
    ent[i].dx = dx; ent[i].dy = dy;
    ent[i].vivo = 1; ent[i].espera = 0;
}

static int simbolo(void *ctx, char c, int x, int y)
{
    (void)ctx;
    switch (c) {
        case '@': jx = x; jy = y; break;
        case 'o': criar(ANDARILHO, x, y, 0, 0); break;
        case 'P': criar(PATRULHEIRO, x, y, 1, 0); break;
        case 'C': criar(PERSEGUIDOR, x, y, 0, 0); break;
        case 'S': criar(ATIRADOR, x, y, 0, 0); break;
    }
    return CHAO;
}

static int sinal(int v) { return (v > 0) - (v < 0); }

static int ocupado(const Mapa *m, int x, int y)
{
    int i;
    if (mapa_bloqueia(m, x, y)) return 1;
    for (i = 0; i < n_ent; i++)
        if (ent[i].vivo && ent[i].tipo != TIRO && ent[i].x == x && ent[i].y == y)
            return 1;
    return 0;
}

/* Um passo de cada entidade. Devolve 1 se alguém tocou no jogador. */
static int atualizar(Mapa *m)
{
    int i, ferido = 0;
    for (i = 0; i < n_ent; i++) {
        Ent *e = &ent[i];
        if (!e->vivo || --e->espera > 0) continue;
        switch (e->tipo) {
        case ANDARILHO:                     /* vagueia ao acaso */
            e->espera = 6;
            e->dx = rng_intervalo(-1, 1);
            e->dy = e->dx ? 0 : rng_intervalo(-1, 1);
            break;
        case PATRULHEIRO:                   /* anda reto; na parede, dá meia-volta */
            e->espera = 4;
            if (ocupado(m, e->x + e->dx, e->y + e->dy)) { e->dx = -e->dx; e->dy = -e->dy; }
            break;
        case PERSEGUIDOR:                   /* só persegue quem consegue ver */
            e->espera = 5;
            if (linha_de_visao(m, e->x, e->y, jx, jy)) {
                e->dx = sinal(jx - e->x);
                e->dy = e->dx ? 0 : sinal(jy - e->y);
            } else e->dx = e->dy = 0;
            break;
        case ATIRADOR:                      /* parado; atira quando alinhado */
            e->espera = 15;
            e->dx = e->dy = 0;
            if ((e->x == jx || e->y == jy) && linha_de_visao(m, e->x, e->y, jx, jy))
                criar(TIRO, e->x + sinal(jx - e->x), e->y + sinal(jy - e->y),
                      sinal(jx - e->x), sinal(jy - e->y));
            break;
        case TIRO:
            e->espera = 1;
            if (mapa_bloqueia(m, e->x + e->dx, e->y + e->dy)) { e->vivo = 0; continue; }
            break;
        }
        if (e->tipo == TIRO || !ocupado(m, e->x + e->dx, e->y + e->dy)) {
            e->x += e->dx;
            e->y += e->dy;
        }
        if (e->x == jx && e->y == jy) {
            ferido = 1;
            if (e->tipo == TIRO) e->vivo = 0;
        }
    }
    return ferido;
}

int main(void)
{
    static const unsigned int GLIFO[] = { 'o', 'P', 'C', 'S', '*' };
    static const int COR[] = { VERDE_CLARO, CIANO_CLARO, VERMELHO_CLARO, MAGENTA_CLARO, AMARELO };
    Mapa m = { 0 };
    int vida = 10, ouro = 0, chaves = 0, invencivel = 0, i;
    double proximo;
    if (!mapa_carregar(&m, "arena.txt", TILES, LEGENDA, simbolo, NULL)) return 1;
    term_iniciar();
    proximo = term_tempo();
    while (vida > 0) {
        int tecla, nx = jx, ny = jy;
        while ((tecla = term_tecla()) != TECLA_NENHUMA) {
            if (tecla == 'q') vida = 0;
            if (tecla == TECLA_CIMA) ny--;
            if (tecla == TECLA_BAIXO) ny++;
            if (tecla == TECLA_ESQUERDA) nx--;
            if (tecla == TECLA_DIREITA) nx++;
        }
        if (mapa_tile(&m, nx, ny) == PORTA_TRANCADA && chaves > 0) {
            chaves--;
            mapa_por(&m, nx, ny, PORTA);
        }
        if (!mapa_bloqueia(&m, nx, ny)) { jx = nx; jy = ny; }
        if (mapa_tile(&m, jx, jy) == OURO)  { ouro += 10; mapa_por(&m, jx, jy, CHAO); }
        if (mapa_tile(&m, jx, jy) == CHAVE) { chaves++;   mapa_por(&m, jx, jy, CHAO); }

        while (term_tempo() >= proximo) {
            if (atualizar(&m) && invencivel == 0) { vida--; invencivel = 20; }
            if (invencivel > 0) invencivel--;
            proximo += 1.0 / 20;
        }
        tela_limpar();
        {
            Camera cam = { 0, 0, 15, 2, 50, 15 };
            int tx, ty;
            mapa_desenhar(&m, &cam, 0);
            for (i = 0; i < n_ent; i++)
                if (ent[i].vivo && camera_na_tela(&cam, ent[i].x, ent[i].y, &tx, &ty))
                    tela_por(tx, ty, GLIFO[ent[i].tipo], COR[ent[i].tipo], PRETO);
            if (camera_na_tela(&cam, jx, jy, &tx, &ty))
                tela_por(tx, ty, '@', invencivel % 4 < 2 ? BRANCO : VERMELHO, PRETO);
        }
        tela_textof(15, 18, CINZA, PRETO, "Vida %d   Ouro %d   Chaves %d   (q sai)", vida, ouro, chaves);
        tela_texto(15, 20, VERDE_CLARO, PRETO, "o andarilho");
        tela_texto(28, 20, CIANO_CLARO, PRETO, "P patrulheiro");
        tela_texto(43, 20, VERMELHO_CLARO, PRETO, "C perseguidor");
        tela_texto(58, 20, MAGENTA_CLARO, PRETO, "S atirador");
        tela_mostrar();
        term_esperar_ate(proximo);
    }
    term_encerrar();
    mapa_destruir(&m);
    printf("Fim de jogo! Ouro: %d\n", ouro);
    return 0;
}
