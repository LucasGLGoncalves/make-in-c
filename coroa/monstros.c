/*
 * monstros.c — combate e o turno dos monstros.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "rng.h"
#include "caminho.h"
#include "coroa.h"

/* Rola um ataque. Devolve 1 se acertou, e o dano em *dano. */
int combate_rolar(int ataque, int defesa, int *dano)
{
    int chance = 75 + (ataque - defesa) * 3;
    if (chance < 15) chance = 15;
    if (chance > 95) chance = 95;
    if (!rng_chance(chance)) return 0;
    *dano = rng_intervalo(1, ataque) - defesa / 2;
    if (*dano < 1) *dano = 1;
    return 1;
}

void heroi_atacar(Jogo *j, int indice)
{
    Ator *a = &j->atores[indice];
    const MonstroDef *d = &MONSTROS[a->raca];
    int dano;

    a->acordado = 1;
    if (!combate_rolar(heroi_ataque(&j->heroi), d->defesa, &dano)) {
        log_adicionar(&j->log, CINZA, "Você erra o %s.", d->nome);
        return;
    }
    a->hp -= dano;
    if (a->hp > 0) {
        log_adicionar(&j->log, BRANCO, "Você acerta o %s (%d).", d->nome, dano);
        return;
    }
    a->vivo = 0;
    j->heroi.abatidos++;
    log_adicionar(&j->log, VERDE_CLARO, "O %s cai e não se levanta mais.", d->nome);
    heroi_ganhar_xp(j, d->xp);
    if (strcmp(d->id, "rei_corvo") == 0) {
        chao_por(j, item_por_id("coroa"), 1, a->x, a->y);
        log_adicionar(&j->log, AMARELO,
                      "Algo de ferro rola do manto de penas negras...");
    }
}

static void monstro_atacar(Jogo *j, Ator *a)
{
    const MonstroDef *d = &MONSTROS[a->raca];
    int dano;
    if (!combate_rolar(d->ataque, heroi_defesa(&j->heroi), &dano)) {
        log_adicionar(&j->log, CINZA, "O %s erra você.", d->nome);
        return;
    }
    j->heroi.hp -= dano;
    log_adicionar(&j->log, VERMELHO_CLARO, "O %s acerta você (%d).", d->nome, dano);
    if (j->heroi.hp <= 0)
        snprintf(j->causa_morte, sizeof j->causa_morte, "%s", d->nome);
}

void monstros_turno(Jogo *j)
{
    Heroi *h = &j->heroi;
    int i;

    /* um único mapa de distâncias serve para todos os monstros */
    caminho_distancias(&j->mapa, j->dist, h->x, h->y, 0, 30);

    for (i = 0; i < j->n_atores && h->hp > 0; i++) {
        Ator *a = &j->atores[i];
        int dx = 0, dy = 0, nx, ny;
        if (!a->vivo || a->raca < 0) continue;      /* NPCs não atacam */

        if (!a->acordado) {                          /* dormindo? */
            if (j->mapa.visivel[a->y * j->mapa.largura + a->x] && rng_chance(60))
                a->acordado = 1;                     /* ...ele viu você! */
            continue;
        }
        if (abs(a->x - h->x) + abs(a->y - h->y) == 1) {
            monstro_atacar(j, a);                    /* colado: ataca */
            continue;
        }
        if (MONSTROS[a->raca].erratico && rng_chance(50)) {
            static const int DX[4] = { 1, -1, 0, 0 }, DY[4] = { 0, 0, 1, -1 };
            int d = rng_intervalo(0, 3);
            dx = DX[d];
            dy = DY[d];
        } else if (!caminho_descer(&j->mapa, j->dist, a->x, a->y, 0, &dx, &dy)) {
            continue;                                /* não há caminho */
        }
        nx = a->x + dx;
        ny = a->y + dy;
        if (!mapa_bloqueia(&j->mapa, nx, ny) && ator_em(j, nx, ny) < 0 &&
            !(nx == h->x && ny == h->y)) {
            a->x = nx;
            a->y = ny;
        }
    }
}
