/*
 * locais.c — tiles, leitura dos arquivos de local e a chuva.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "rng.h"
#include "dados.h"
#include "dialogo.h"
#include "noir.h"

const TileDef TILES[] = {
    { ' ',            PRETO,          PRETO,   T_SOLIDO | T_OPACO,  "vazio"   },
    { G_BLOCO,        CINZA_ESCURO,   PRETO,   T_SOLIDO | T_OPACO,  "parede"  },
    { G_PONTO,        CINZA_ESCURO,   PRETO,   0,                   "piso"    },
    { G_PONTO,        AZUL,           PRETO,   T_EXTERNO,           "rua"     },
    { G_SOMBRA_LEVE,  VERMELHO,       PRETO,   0,                   "carpete" },
    { G_SOMBRA_MEDIA, MARROM,         PRETO,   T_OPACO,             "porta"   },
    { G_PI,           MARROM,         PRETO,   T_SOLIDO,            "balcão"  },
    { G_SOMBRA_LEVE,  AZUL,           PRETO,   T_SOLIDO | T_EXTERNO,"janela"  },
    { G_QUADRADO,     MARROM,         PRETO,   T_SOLIDO,            "mesa"    },
    { G_SOMBRA_FORTE, MAGENTA_CLARO,  MAGENTA, T_SOLIDO | T_OPACO | T_NEON, "letreiro" },
    { G_APROX,        AZUL,           PRETO,   T_SOLIDO | T_EXTERNO,"mar"     },
    { G_TRI_BAIXO,    AMARELO,        PRETO,   T_EXTERNO,           "ponto de táxi" },
    { G_SOMBRA_LEVE,  MARROM,         PRETO,   0,                   "palco"   },
    { G_SOMBRA_MEDIA, BRANCO,         AZUL,    T_SOLIDO,            "cama"    },
    { G_TRIPLO,       MARROM,         PRETO,   T_SOLIDO,            "estante" },
    { G_PI,           BRANCO,         PRETO,   T_SOLIDO,            "piano"   }
};

/* tile i <-> caractere i; letras minúsculas são pessoas e objetos */
static const char LEGENDA[] = " #.,=+|WMN~S_BLP";

static int cor_por_nome(const char *nome)
{
    static const char *const CORES[16] = {
        "PRETO", "AZUL", "VERDE", "CIANO", "VERMELHO", "MAGENTA", "MARROM",
        "CINZA", "CINZA_ESCURO", "AZUL_CLARO", "VERDE_CLARO", "CIANO_CLARO",
        "VERMELHO_CLARO", "MAGENTA_CLARO", "AMARELO", "BRANCO"
    };
    int i;
    for (i = 0; i < 16; i++)
        if (strcmp(nome, CORES[i]) == 0) return i;
    return CINZA;
}

/* "o|Otávio Brandão|U+263A|VERMELHO_CLARO|otavio|condição" */
static void ler_coisa(Caso *c, char *linha)
{
    char *campo[6];
    Coisa *k;
    int n = dados_separar(linha, '|', campo, 6);
    if (n < 5 || c->n_coisas >= MAX_COISAS) return;
    k = &c->coisas[c->n_coisas++];
    memset(k, 0, sizeof *k);
    k->letra = campo[0][0];
    snprintf(k->nome, sizeof k->nome, "%s", campo[1]);
    k->glifo = (campo[2][0] == 'U' && campo[2][1] == '+')
             ? (unsigned int)strtoul(campo[2] + 2, NULL, 16)
             : (unsigned char)campo[2][0];
    k->cor = cor_por_nome(campo[3]);
    snprintf(k->no, sizeof k->no, "%s", campo[4]);
    snprintf(k->condicao, sizeof k->condicao, "%s", n > 5 ? campo[5] : "");
    k->x = k->y = -1;
}

/* Posições especiais achadas no mapa: '@' (início do jogo) e
 * '*' (onde o táxi deixa a detetive). -1 = não existe. */
static int inicio_x, inicio_y, taxi_x, taxi_y;

/* Chamada pelo mapa_ler para letras fora da legenda. */
static int simbolo(void *ctx, char letra, int x, int y)
{
    Caso *c = ctx;
    int i;
    if (letra == '@') { inicio_x = x; inicio_y = y; return NT_PISO; }
    if (letra == '*') { taxi_x = x;   taxi_y = y;   return NT_RUA;  }
    for (i = 0; i < c->n_coisas; i++)
        if (c->coisas[i].letra == letra) {
            c->coisas[i].x = x;
            c->coisas[i].y = y;
        }
    return NT_PISO;
}

int local_carregar(Caso *c, const char *nome, int de_taxi)
{
    char caminho[64], linha[256];
    FILE *f;
    int ok;

    snprintf(caminho, sizeof caminho, "dados/locais/%s.txt", nome);
    f = fopen(caminho, "r");
    if (f == NULL) return 0;

    snprintf(c->local, sizeof c->local, "%s", nome);
    dados_ler_linha(f, c->nome_local, sizeof c->nome_local);  /* 1ª linha */
    c->n_coisas = 0;
    while (dados_ler_linha(f, linha, sizeof linha) && strcmp(linha, "MAPA") != 0)
        ler_coisa(c, linha);

    mapa_destruir(&c->mapa);
    inicio_x = inicio_y = taxi_x = taxi_y = -1;
    ok = mapa_ler(&c->mapa, f, TILES, LEGENDA, simbolo, c);
    fclose(f);

    if ((de_taxi && taxi_x >= 0) || inicio_x < 0) { c->x = taxi_x;   c->y = taxi_y;   }
    else                                          { c->x = inicio_x; c->y = inicio_y; }

    c->cam.tela_x = 0;  c->cam.tela_y = 0;
    c->cam.largura = TELA_L;  c->cam.altura = VISTA_A;
    chuva_iniciar(c);
    return ok && c->x >= 0;
}

int coisa_presente(Caso *c, int i)
{
    const Coisa *k = &c->coisas[i];
    return k->x >= 0 && dialogo_condicao(k->condicao, caso_condicao, c);
}

int coisa_em(Caso *c, int x, int y)
{
    int i;
    for (i = 0; i < c->n_coisas; i++)
        if (c->coisas[i].x == x && c->coisas[i].y == y && coisa_presente(c, i))
            return i;
    return -1;
}

static void gota_nova(Caso *c, Gota *g, int no_topo)
{
    g->x = rng_intervalo(0, c->mapa.largura - 1);
    g->y = no_topo ? 0 : rng_intervalo(0, c->mapa.altura - 1);
}

void chuva_iniciar(Caso *c)
{
    int i;
    for (i = 0; i < MAX_GOTAS; i++) gota_nova(c, &c->gotas[i], 0);
}

void chuva_atualizar(Caso *c)
{
    int i;
    for (i = 0; i < MAX_GOTAS; i++) {
        Gota *g = &c->gotas[i];
        g->y++;                               /* cai uma linha...          */
        if (rng_chance(30)) g->x--;           /* ...e o vento empurra       */
        if (g->y >= c->mapa.altura || g->x < 0 || rng_chance(4))
            gota_nova(c, g, 1);               /* sumiu: nasce outra no alto */
    }
}
