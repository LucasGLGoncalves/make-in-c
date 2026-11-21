/*
 * niveis.c — a tabela de tiles e a leitura dos arquivos de nível.
 */
#include <stdio.h>
#include <string.h>
#include "tela.h"
#include "dados.h"
#include "templo.h"

#define G_CHICOTE 0x2310u   /* ⌐ (169) */

const TileDef TILES[] = {
    /* glifo          frente          fundo    flags                 nome       */
    { ' ',            CINZA,          PRETO,   0,                    "chão"      },
    { G_BLOCO,        MARROM,         PRETO,   T_SOLIDO | T_OPACO,   "parede"    },
    { G_SOMBRA_FORTE, MARROM,         PRETO,   T_SOLIDO | T_OPACO,   "parede rachada" },
    { G_PAUS,         VERDE,          PRETO,   T_SOLIDO,             "mato"      },
    { G_SOMBRA_MEDIA, CIANO_CLARO,    AZUL,    T_SOLIDO | T_OPACO,   "porta"     },
    { G_APROX,        AZUL_CLARO,     AZUL,    T_SOLIDO,             "água"      },
    { G_TRI_CIMA,     VERMELHO_CLARO, VERMELHO, T_LAVA,              "lava"      },
    { G_OUROS,        CIANO_CLARO,    PRETO,   0,                    "gema"      },
    { G_CHICOTE,      BRANCO,         PRETO,   0,                    "chicote"   },
    { G_FEMININO,     AMARELO,        PRETO,   0,                    "chave"     },
    { G_SOL,          MAGENTA_CLARO,  PRETO,   0,                    "orbe"      },
    { '$',            AMARELO,        PRETO,   0,                    "tesouro"   },
    { G_TRIPLO,       PRETO,          MAGENTA_CLARO, 0,              "saída"     },
    { G_FI,           AMARELO,        VERMELHO, 0,                   "ídolo"     }
};

/* O caractere na posição i representa o tile i do enum. */
static const char LEGENDA[] = " #X+D~^*wko$EI";

/* Chamada por mapa_ler() para cada símbolo fora da legenda. */
static int simbolo(void *ctx, char c, int x, int y)
{
    Jogo *j = ctx;
    if (c == '@') {
        j->jog.x = x;
        j->jog.y = y;
    } else if (c >= '1' && c <= '3' && j->n_monstros < MAX_MONSTROS) {
        Monstro *m = &j->monstros[j->n_monstros++];
        m->x = x;
        m->y = y;
        m->tipo = c - '1';
        m->vivo = 1;
        m->espera = 20 + j->n_monstros % 10;   /* não saem todos juntos */
    }
    return TILE_CHAO;                      /* embaixo de tudo há chão */
}

int nivel_carregar(Jogo *j, int numero)
{
    char caminho[64];
    FILE *f;
    int ok;

    sprintf(caminho, "niveis/nivel%d.txt", numero);
    f = fopen(caminho, "r");
    if (f == NULL) return 0;

    /* a primeira linha do arquivo é o título do nível */
    if (!dados_ler_linha_crua(f, j->titulo, sizeof j->titulo))
        j->titulo[0] = '\0';

    mapa_destruir(&j->mapa);
    j->n_monstros = 0;
    ok = mapa_ler(&j->mapa, f, TILES, LEGENDA, simbolo, j);
    fclose(f);

    j->nivel = numero;
    j->aviso[0] = '\0';
    return ok;
}
