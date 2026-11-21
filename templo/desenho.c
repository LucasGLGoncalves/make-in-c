/*
 * desenho.c — campo de jogo, monstros, jogador e painel lateral.
 */
#include "tela.h"
#include "ui.h"
#include "templo.h"

static const unsigned int GLIFO_MONSTRO[3] = { G_A_TREMA, G_O_TREMA, G_OMEGA };
static const int          COR_MONSTRO[3]   = { VERMELHO_CLARO, VERDE_CLARO,
                                               MAGENTA_CLARO };

static void painel_item(int y, const char *rotulo, long valor, int cor)
{
    tela_texto(PAINEL_X + 2, y, AMARELO, AZUL, rotulo);
    tela_retangulo(PAINEL_X + 2, y + 1, 12, 1, ' ', cor, CINZA_ESCURO);
    tela_textof(PAINEL_X + 3, y + 1, cor, CINZA_ESCURO, "%ld", valor);
}

static void desenhar_painel(const Jogo *j)
{
    const Jogador *p = &j->jog;
    tela_retangulo(PAINEL_X, 0, TELA_L - PAINEL_X, CAMPO_A, ' ', CINZA, AZUL);
    painel_item(1,  "Pontos",   p->pontos,   BRANCO);
    painel_item(4,  "Nível",    j->nivel,    BRANCO);
    painel_item(7,  "Gemas",    p->gemas,    p->gemas < 5 ? VERMELHO_CLARO : BRANCO);
    painel_item(10, "Chicotes", p->chicotes, BRANCO);
    painel_item(13, "Chaves",   p->chaves,   BRANCO);
    painel_item(16, "Orbes",    p->orbes,    BRANCO);
    tela_texto(PAINEL_X + 2, 19, CIANO_CLARO, AZUL, "C chicote");
    tela_texto(PAINEL_X + 2, 20, CIANO_CLARO, AZUL, "T teleporte");
    tela_texto(PAINEL_X + 2, 21, CIANO_CLARO, AZUL, "P pausa");
}

void desenhar_jogo(const Jogo *j)
{
    Camera cam = { 0, 0, 0, 0, CAMPO_L, CAMPO_A };
    int i;

    mapa_desenhar(&j->mapa, &cam, 0);             /* 1. o cenário   */

    for (i = 0; i < j->n_monstros; i++) {         /* 2. os monstros */
        const Monstro *m = &j->monstros[i];
        if (m->vivo)
            tela_por(m->x, m->y, GLIFO_MONSTRO[m->tipo],
                     COR_MONSTRO[m->tipo], PRETO);
    }
    tela_por(j->jog.x, j->jog.y, G_ROSTO_CHEIO, AMARELO, PRETO); /* 3. você */

    desenhar_painel(j);                           /* 4. o painel    */

    tela_retangulo(0, CAMPO_A, TELA_L, 1, ' ', CINZA, PRETO);   /* 5. aviso */
    if (j->aviso_ticks > 0)
        tela_texto(1, CAMPO_A, AMARELO, PRETO, j->aviso);
    else
        tela_textof(1, CAMPO_A, CINZA_ESCURO, PRETO, "%s", j->titulo);
}
