/*
 * mundo.c — os tiles, a aldeia (lida de arquivo) e a masmorra (gerada).
 */
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "rng.h"
#include "gerador.h"
#include "caminho.h"
#include "fov.h"
#include "coroa.h"

const TileDef TILES[] = {
    { ' ',            CINZA,       PRETO, T_SOLIDO | T_OPACO, "rocha"            },
    { G_SOMBRA_MEDIA, CINZA,       PRETO, T_SOLIDO | T_OPACO, "parede"           },
    { G_PONTO,        CINZA,       PRETO, 0,                  "chão"             },
    { '+',            MARROM,      PRETO, T_OPACO,            "porta"            },
    { '>',            BRANCO,      PRETO, 0,                  "escada que desce" },
    { '<',            BRANCO,      PRETO, 0,                  "escada que sobe"  },
    { '.',            VERDE,       PRETO, 0,                  "grama"            },
    { G_PAUS,         VERDE,       PRETO, T_SOLIDO | T_OPACO, "árvore"           },
    { G_APROX,        AZUL_CLARO,  AZUL,  T_SOLIDO,           "rio"              },
    { G_SOMBRA_LEVE,  MARROM,      PRETO, 0,                  "estrada"          },
    { G_BLOCO,        MARROM,      PRETO, T_SOLIDO | T_OPACO, "parede de madeira"},
    { G_PONTO,        MARROM,      PRETO, 0,                  "assoalho"         },
    { G_PI,           AMARELO,     PRETO, T_SOLIDO,           "balcão"           },
    { G_CIRCULO,      CIANO_CLARO, AZUL,  T_SOLIDO,           "poço"             }
};

/* posição i = caractere que representa o tile i no arquivo da aldeia */
static const char LEGENDA_ALDEIA[] = "\x01#,+><.T~:H=|O";

int ator_em(const Jogo *j, int x, int y)
{
    int i;
    for (i = 0; i < j->n_atores; i++)
        if (j->atores[i].vivo && j->atores[i].x == x && j->atores[i].y == y)
            return i;
    return -1;
}

int chao_em(const Jogo *j, int x, int y)
{
    int i;
    for (i = j->n_chao - 1; i >= 0; i--)
        if (j->chao[i].x == x && j->chao[i].y == y) return i;
    return -1;
}

void chao_por(Jogo *j, int def, int qtd, int x, int y)
{
    ItemChao *c;
    if (j->n_chao >= MAX_CHAO || def < 0) return;
    c = &j->chao[j->n_chao++];
    c->def = def; c->qtd = qtd; c->x = x; c->y = y;
}

static Ator *novo_ator(Jogo *j, int x, int y)
{
    Ator *a;
    if (j->n_atores >= MAX_ATORES) return NULL;
    a = &j->atores[j->n_atores++];
    memset(a, 0, sizeof *a);
    a->x = x; a->y = y;
    a->raca = -1; a->npc = -1;
    a->vivo = 1;
    return a;
}

static int simbolo_aldeia(void *ctx, char c, int x, int y)
{
    Jogo *j = ctx;
    int npc = npc_por_simbolo(c);
    if (c == '@') {
        j->heroi.x = x;
        j->heroi.y = y;
        return TL_GRAMA;
    }
    if (npc >= 0) {
        Ator *a = novo_ator(j, x, y);
        if (a != NULL) a->npc = npc;
        return (c == 'g') ? TL_ESTRADA : TL_ASSOALHO;
    }
    return TL_GRAMA;
}

static int carregar_aldeia(Jogo *j, int chegando_da_masmorra)
{
    int x, y;
    if (!mapa_carregar(&j->mapa, "dados/aldeia.txt", TILES, LEGENDA_ALDEIA,
                       simbolo_aldeia, j))
        return 0;
    if (chegando_da_masmorra)            /* aparece na entrada da abadia */
        for (y = 0; y < j->mapa.altura; y++)
            for (x = 0; x < j->mapa.largura; x++)
                if (mapa_tile(&j->mapa, x, y) == TL_DESCE) {
                    j->heroi.x = x;
                    j->heroi.y = y;
                }
    fov_revelar_tudo(&j->mapa);          /* de dia, a aldeia toda é visível */
    return 1;
}

static void ponto_livre(const Jogo *j, int *x, int *y)
{
    do {
        *x = rng_intervalo(1, j->mapa.largura - 2);
        *y = rng_intervalo(1, j->mapa.altura - 2);
    } while (mapa_bloqueia(&j->mapa, *x, *y) || ator_em(j, *x, *y) >= 0 ||
             (*x == j->heroi.x && *y == j->heroi.y));
}

static int sortear_monstro(int prof)
{
    int candidatos[MAX_MONSTRO_DEFS], n = 0, i;
    for (i = 0; i < N_MONSTROS; i++)
        if (MONSTROS[i].prof_min > 0 && MONSTROS[i].prof_min <= prof &&
            prof <= MONSTROS[i].prof_max)
            candidatos[n++] = i;
    return n ? candidatos[rng_intervalo(0, n - 1)] : -1;
}

static int sortear_item(int prof)
{
    int candidatos[MAX_ITEM_DEFS], n = 0, i;
    for (i = 0; i < N_ITENS; i++)
        if (ITENS[i].prof_min > 0 && ITENS[i].prof_min <= prof)
            candidatos[n++] = i;
    return n ? candidatos[rng_intervalo(0, n - 1)] : -1;
}

static void criar_monstro(Jogo *j, int raca, int x, int y)
{
    Ator *a = novo_ator(j, x, y);
    if (a == NULL || raca < 0) return;
    a->raca = raca;
    a->hp = a->hp_max = MONSTROS[raca].hp;
}

static void gerar_nivel(Jogo *j, int prof, int chegando_de_cima)
{
    Sala salas[14];
    int n, i, x, y, sx, sy, dx = 0, dy = 0;
    Heroi *h = &j->heroi;

    mapa_criar(&j->mapa, MASMORRA_L, MASMORRA_A, TILES);

    if (prof == 3) {                     /* o nível 3 é uma caverna natural */
        int *dist = malloc(sizeof(int) * MASMORRA_L * MASMORRA_A), melhor = -1;
        gerar_caverna(&j->mapa, TL_PAREDE, TL_CHAO, 45, 4);
        ponto_livre(j, &sx, &sy);
        caminho_distancias(&j->mapa, dist, sx, sy, 0, 0);
        for (i = 0; i < MASMORRA_L * MASMORRA_A; i++)   /* a mais distante */
            if (dist[i] < CAMINHO_LONGE && dist[i] > melhor) {
                melhor = dist[i];
                dx = i % MASMORRA_L;
                dy = i / MASMORRA_L;
            }
        free(dist);
        n = 0;
    } else {
        do {                             /* raríssimo, mas: menos de 2 salas? */
            n = gerar_masmorra(&j->mapa, TL_PAREDE, TL_CHAO, salas, 12);
        } while (n < 2);
        sala_centro(&salas[0], &sx, &sy);
        sala_centro(&salas[n - 1], &dx, &dy);
    }

    mapa_por(&j->mapa, sx, sy, TL_SOBE);
    if (prof < PROFUNDIDADE_FINAL) mapa_por(&j->mapa, dx, dy, TL_DESCE);
    if (chegando_de_cima) { h->x = sx; h->y = sy; }
    else                  { h->x = dx; h->y = dy; }

    /* monstros: mais fundo = mais monstros */
    for (i = 0; i < 4 + prof * 2; i++) {
        ponto_livre(j, &x, &y);
        if (abs(x - h->x) + abs(y - h->y) < 8) continue;  /* longe da escada */
        criar_monstro(j, sortear_monstro(prof), x, y);
    }
    /* itens e ouro */
    for (i = 0; i < 6 + rng_intervalo(0, 3); i++) {
        ponto_livre(j, &x, &y);
        if (rng_chance(35))
            chao_por(j, item_por_id("ouro"), rng_intervalo(5, 15) * prof, x, y);
        else
            chao_por(j, sortear_item(prof), 1, x, y);
    }
    /* o último nível guarda o trono do Rei Corvo */
    if (prof == PROFUNDIDADE_FINAL && n > 0) {
        int rei;
        for (rei = 0; rei < N_MONSTROS; rei++)
            if (strcmp(MONSTROS[rei].id, "rei_corvo") == 0) break;
        criar_monstro(j, rei, dx, dy);
        for (i = 0; i < 2; i++)
            if (sala_ponto_livre(&j->mapa, &salas[n - 1], &x, &y) &&
                ator_em(j, x, y) < 0)
                criar_monstro(j, sortear_monstro(prof), x, y);
    }
}

int mundo_ir_para(Jogo *j, int profundidade, int vindo_de_cima)
{
    int ok = 1;
    mapa_destruir(&j->mapa);
    j->n_atores = 0;
    j->n_chao = 0;
    j->profundidade = profundidade;

    if (profundidade == 0) ok = carregar_aldeia(j, !vindo_de_cima);
    else                   gerar_nivel(j, profundidade, vindo_de_cima);
    if (!ok) return 0;

    free(j->dist);
    j->dist = malloc(sizeof(int) * (size_t)(j->mapa.largura * j->mapa.altura));
    j->cam.tela_x = 0;
    j->cam.tela_y = 0;
    j->cam.largura = VISTA_L;
    j->cam.altura = VISTA_A;
    if (profundidade > j->heroi.prof_max) j->heroi.prof_max = profundidade;
    return j->dist != NULL;
}
