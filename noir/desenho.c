/*
 * desenho.c — o local, a chuva, o neon, as pessoas e a barra de status.
 */
#include <stdio.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "noir.h"

/* Letreiros: acesos ou apagados conforme o relógio, com uma falha às vezes. */
static void desenhar_neon(const Caso *c)
{
    int i, j, tx, ty;
    int aceso = ((int)(term_tempo() * 1.5) % 4) != 0;   /* apaga 1/4 do tempo */
    for (j = 0; j < c->mapa.altura; j++)
        for (i = 0; i < c->mapa.largura; i++)
            if ((mapa_flags(&c->mapa, i, j) & T_NEON) &&
                camera_na_tela(&c->cam, i, j, &tx, &ty)) {
                Celula *cel = tela_celula(tx, ty);
                cel->frente = aceso ? MAGENTA_CLARO : MAGENTA;
                cel->fundo  = aceso ? MAGENTA : PRETO;
            }
}

static void desenhar_chuva(const Caso *c)
{
    int i, tx, ty;
    for (i = 0; i < MAX_GOTAS; i++) {
        const Gota *g = &c->gotas[i];
        unsigned int f = mapa_flags(&c->mapa, g->x, g->y);
        if (!(f & T_EXTERNO)) continue;               /* sob um teto: seco */
        if (!camera_na_tela(&c->cam, g->x, g->y, &tx, &ty)) continue;
        /* atrás do vidro ou do mar a gota é mais escura */
        tela_por(tx, ty, '/', (f & T_SOLIDO) ? AZUL : CIANO, PRETO);
    }
}

void desenhar_caso(Caso *c)
{
    char hora[16];
    int i, tx, ty;

    tela_limpar();
    camera_centralizar(&c->cam, &c->mapa, c->x, c->y);
    mapa_desenhar(&c->mapa, &c->cam, 0);
    desenhar_neon(c);
    desenhar_chuva(c);

    for (i = 0; i < c->n_coisas; i++) {
        const Coisa *k = &c->coisas[i];
        if (!coisa_presente(c, i)) continue;
        if (camera_na_tela(&c->cam, k->x, k->y, &tx, &ty))
            tela_por(tx, ty, k->glifo, k->cor, PRETO);
    }
    if (camera_na_tela(&c->cam, c->x, c->y, &tx, &ty))
        tela_por(tx, ty, G_ROSTO_CHEIO, BRANCO, PRETO);

    /* barra de status */
    caso_hora(c, hora);
    tela_retangulo(0, VISTA_A, TELA_L, 1, ' ', PRETO, CINZA);
    tela_texto_max(1, VISTA_A, PRETO, CINZA, c->nome_local, 34);
    tela_textof(37, VISTA_A, c->minutos > PRAZO_MINUTOS - 60 ? VERMELHO : PRETO,
                CINZA, "%s", hora);
    tela_textof(48, VISTA_A, PRETO, CINZA, "$%d", c->dinheiro);
    tela_textof(55, VISTA_A, PRETO, CINZA, "Pistas %d/%d", c->n_pistas,
                caso_total_pistas());
    tela_texto(68, VISTA_A, AZUL, CINZA, "N caderno");
    log_desenhar(&c->log, 1, LOG_Y, TELA_L - 2, TELA_A - LOG_Y);
}
