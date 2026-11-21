/*
 * desenho.c — mapa, itens, atores, painel e registro de mensagens.
 */
#include <stdio.h>
#include "tela.h"
#include "ui.h"
#include "coroa.h"

static void desenhar_painel(const Jogo *j)
{
    const Heroi *h = &j->heroi;
    int arma = slot_ocupado(h, SLOT_MAO), corpo = slot_ocupado(h, SLOT_CORPO);
    int x = PAINEL_X + 1;

    tela_retangulo(PAINEL_X, 0, TELA_L - PAINEL_X, VISTA_A, ' ', CINZA, PRETO);
    tela_retangulo(PAINEL_X, 0, 1, VISTA_A, G_V, CINZA_ESCURO, PRETO);

    tela_texto_max(x + 1, 0, AMARELO, PRETO, h->nome, 17);
    tela_textof(x + 1, 1, CINZA, PRETO, "Nível %d", h->nivel);
    tela_textof(x + 1, 3, CINZA, PRETO, "Vida %d/%d", h->hp, h->hp_max);
    ui_barra(x + 1, 4, 17, h->hp, h->hp_max,
             h->hp * 3 < h->hp_max ? VERMELHO_CLARO : VERDE_CLARO, CINZA_ESCURO);
    tela_textof(x + 1, 5, CINZA, PRETO, "XP %d/%d", h->xp, h->nivel * 20);
    ui_barra(x + 1, 6, 17, h->xp, h->nivel * 20, AMARELO, CINZA_ESCURO);
    tela_textof(x + 1, 8, CINZA, PRETO, "Ataque  %d", heroi_ataque(h));
    tela_textof(x + 1, 9, CINZA, PRETO, "Defesa  %d", heroi_defesa(h));
    tela_textof(x + 1, 10, AMARELO, PRETO, "Ouro    %d", h->ouro);
    if (j->profundidade == 0)
        tela_texto(x + 1, 12, VERDE_CLARO, PRETO, "Vale Cinzento");
    else
        tela_textof(x + 1, 12, CIANO_CLARO, PRETO, "Abadia, nível %d", j->profundidade);
    if (arma >= 0)
        tela_texto_max(x + 1, 14, BRANCO, PRETO, ITENS[h->mochila[arma].def].nome, 17);
    if (corpo >= 0)
        tela_texto_max(x + 1, 15, BRANCO, PRETO, ITENS[h->mochila[corpo].def].nome, 17);
    tela_texto(x + 1, 17, CINZA_ESCURO, PRETO, "i mochila  ? ajuda");
}

void desenhar_jogo(Jogo *j)
{
    const Heroi *h = &j->heroi;
    int i, tx, ty, fov = (j->profundidade > 0);

    tela_limpar();                       /* nada de restos de janelas antigas */
    camera_centralizar(&j->cam, &j->mapa, h->x, h->y);
    mapa_desenhar(&j->mapa, &j->cam, fov);

    for (i = 0; i < j->n_chao; i++) {             /* itens no chão */
        const ItemChao *c = &j->chao[i];
        if (!j->mapa.visivel[c->y * j->mapa.largura + c->x]) continue;
        if (camera_na_tela(&j->cam, c->x, c->y, &tx, &ty))
            tela_por(tx, ty, ITENS[c->def].glifo, ITENS[c->def].cor, PRETO);
    }
    for (i = 0; i < j->n_atores; i++) {           /* monstros e moradores */
        const Ator *a = &j->atores[i];
        if (!a->vivo || !j->mapa.visivel[a->y * j->mapa.largura + a->x]) continue;
        if (!camera_na_tela(&j->cam, a->x, a->y, &tx, &ty)) continue;
        if (a->npc >= 0)
            tela_por(tx, ty, npc_glifo(a->npc), npc_cor(a->npc), PRETO);
        else
            tela_por(tx, ty, MONSTROS[a->raca].glifo, MONSTROS[a->raca].cor, PRETO);
    }
    if (camera_na_tela(&j->cam, h->x, h->y, &tx, &ty))   /* o herói */
        tela_por(tx, ty, '@', BRANCO, PRETO);

    desenhar_painel(j);
    tela_retangulo(0, LOG_Y, TELA_L, 1, G_H, CINZA_ESCURO, PRETO);
    log_desenhar(&j->log, 1, LOG_Y + 1, TELA_L - 2, TELA_A - LOG_Y - 1);
}
