/*
 * monstros.c — os habitantes do templo e a sua "inteligência".
 */
#include "templo.h"

/* Valores por tipo de monstro: rastejante, guardião, serpente. */
static const int PASSO_TICKS[3] = { 9, 6, 4 };   /* menor = mais rápido */
static const int DANO[3]        = { 1, 2, 3 };   /* gemas perdidas      */
static const int PONTOS[3]      = { 10, 20, 30 };

int monstro_em(const Jogo *j, int x, int y)
{
    int i;
    for (i = 0; i < j->n_monstros; i++)
        if (j->monstros[i].vivo && j->monstros[i].x == x && j->monstros[i].y == y)
            return i;
    return -1;
}

void monstro_atingir_jogador(Jogo *j, Monstro *m)
{
    j->jog.gemas  -= DANO[m->tipo];
    j->jog.pontos += PONTOS[m->tipo];
    m->vivo = 0;                     /* o monstro se desfaz no ataque */
}

static int sinal(int v)
{
    return (v > 0) - (v < 0);        /* -1, 0 ou 1 */
}

/* Um monstro pode entrar em (x,y)? Lava e outros monstros, não. */
static int livre_para_monstro(const Jogo *j, int x, int y)
{
    unsigned int f = mapa_flags(&j->mapa, x, y);
    if (f & (T_SOLIDO | T_LAVA)) return 0;
    return monstro_em(j, x, y) < 0;
}

void monstros_atualizar(Jogo *j)
{
    int i;
    for (i = 0; i < j->n_monstros; i++) {
        Monstro *m = &j->monstros[i];
        int dx, dy;
        if (!m->vivo) continue;
        if (--m->espera > 0) continue;       /* ainda não é a vez dele */
        m->espera = PASSO_TICKS[m->tipo];

        dx = sinal(j->jog.x - m->x);         /* direção do jogador */
        dy = sinal(j->jog.y - m->y);

        if (m->x + dx == j->jog.x && m->y + dy == j->jog.y) {
            monstro_atingir_jogador(j, m);   /* alcançou o jogador! */
            continue;
        }
        /* tenta a diagonal; se não der, só um dos eixos */
        if (livre_para_monstro(j, m->x + dx, m->y + dy)) {
            m->x += dx; m->y += dy;
        } else if (dx != 0 && livre_para_monstro(j, m->x + dx, m->y)) {
            m->x += dx;
        } else if (dy != 0 && livre_para_monstro(j, m->x, m->y + dy)) {
            m->y += dy;
        }
    }
}
