/*
 * mapa.c — implementação dos mapas de tiles e da câmera.
 */
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "dados.h"
#include "mapa.h"

int mapa_criar(Mapa *m, int largura, int altura, const TileDef *defs)
{
    size_t n = (size_t)largura * (size_t)altura;
    m->largura = largura;
    m->altura  = altura;
    m->defs    = defs;
    m->tiles   = calloc(n, 1);
    m->visivel = calloc(n, 1);
    m->visto   = calloc(n, 1);
    if (!m->tiles || !m->visivel || !m->visto) {
        mapa_destruir(m);
        return 0;
    }
    return 1;
}

void mapa_destruir(Mapa *m)
{
    free(m->tiles);
    free(m->visivel);
    free(m->visto);
    m->tiles = m->visivel = m->visto = NULL;
    m->largura = m->altura = 0;
}

int mapa_dentro(const Mapa *m, int x, int y)
{
    return x >= 0 && y >= 0 && x < m->largura && y < m->altura;
}

int mapa_tile(const Mapa *m, int x, int y)
{
    if (!mapa_dentro(m, x, y)) return -1;
    return m->tiles[y * m->largura + x];
}

void mapa_por(Mapa *m, int x, int y, int tile)
{
    if (mapa_dentro(m, x, y))
        m->tiles[y * m->largura + x] = (unsigned char)tile;
}

void mapa_preencher(Mapa *m, int tile)
{
    memset(m->tiles, tile, (size_t)m->largura * (size_t)m->altura);
}

unsigned int mapa_flags(const Mapa *m, int x, int y)
{
    int t = mapa_tile(m, x, y);
    if (t < 0) return T_SOLIDO | T_OPACO;  /* fora do mapa = parede */
    return m->defs[t].flags;
}

int mapa_bloqueia(const Mapa *m, int x, int y)
{
    return (mapa_flags(m, x, y) & T_SOLIDO) != 0;
}

int mapa_opaco(const Mapa *m, int x, int y)
{
    return (mapa_flags(m, x, y) & T_OPACO) != 0;
}

/* Lê linhas de 'f' até o fim do arquivo ou até uma linha "FIM".
 * Cada caractere vira um tile segundo a 'legenda': o caractere na
 * posição i da legenda representa o tile de índice i. */
int mapa_ler(Mapa *m, FILE *f, const TileDef *defs, const char *legenda,
             MapaSimbolo desconhecido, void *ctx)
{
    char   buffer[512];
    char **linhas = NULL;
    int    n = 0, capacidade = 0, largura = 0, x, y, ok;

    /* 1ª passada: guarda as linhas para descobrir o tamanho do mapa */
    while (dados_ler_linha_crua(f, buffer, sizeof buffer)) {
        int tamanho = (int)strlen(buffer);
        if (strcmp(buffer, "FIM") == 0) break;
        if (n == capacidade) {
            int nova = capacidade ? capacidade * 2 : 32;
            char **maior = realloc(linhas, (size_t)nova * sizeof *linhas);
            if (maior == NULL) break;
            linhas = maior;
            capacidade = nova;
        }
        linhas[n++] = texto_duplicar(buffer);
        if (tamanho > largura) largura = tamanho;
    }

    /* 2ª passada: cria o mapa e converte cada caractere */
    ok = n > 0 && largura > 0 && mapa_criar(m, largura, n, defs);
    for (y = 0; ok && y < n; y++) {
        for (x = 0; linhas[y][x] != '\0'; x++) {
            char c = linhas[y][x];
            const char *achou = strchr(legenda, c);
            int tile = 0;
            if (achou != NULL)
                tile = (int)(achou - legenda);
            else if (desconhecido != NULL)
                tile = desconhecido(ctx, c, x, y);
            if (tile >= 0) mapa_por(m, x, y, tile);
        }
    }

    for (y = 0; y < n; y++) free(linhas[y]);
    free(linhas);
    return ok;
}

int mapa_carregar(Mapa *m, const char *arquivo, const TileDef *defs,
                  const char *legenda, MapaSimbolo desconhecido, void *ctx)
{
    int ok;
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;
    ok = mapa_ler(m, f, defs, legenda, desconhecido, ctx);
    fclose(f);
    return ok;
}

static int limitar(int v, int minimo, int maximo)
{
    if (v < minimo) return minimo;
    if (v > maximo) return maximo;
    return v;
}

void camera_centralizar(Camera *c, const Mapa *m, int alvo_x, int alvo_y)
{
    if (m->largura <= c->largura)        /* mapa estreito: centraliza */
        c->x = -(c->largura - m->largura) / 2;
    else
        c->x = limitar(alvo_x - c->largura / 2, 0, m->largura - c->largura);

    if (m->altura <= c->altura)
        c->y = -(c->altura - m->altura) / 2;
    else
        c->y = limitar(alvo_y - c->altura / 2, 0, m->altura - c->altura);
}

int camera_na_tela(const Camera *c, int mx, int my, int *tx, int *ty)
{
    int sx = mx - c->x, sy = my - c->y;
    if (sx < 0 || sy < 0 || sx >= c->largura || sy >= c->altura) return 0;
    *tx = c->tela_x + sx;
    *ty = c->tela_y + sy;
    return 1;
}

void mapa_desenhar(const Mapa *m, const Camera *c, int usar_fov)
{
    int i, j;
    for (j = 0; j < c->altura; j++) {
        for (i = 0; i < c->largura; i++) {
            int mx = c->x + i, my = c->y + j;
            int tx = c->tela_x + i, ty = c->tela_y + j;
            const TileDef *d;
            int idx;
            if (!mapa_dentro(m, mx, my)) {
                tela_por(tx, ty, ' ', CINZA, PRETO);
                continue;
            }
            idx = my * m->largura + mx;
            d = &m->defs[m->tiles[idx]];
            if (!usar_fov || m->visivel[idx])
                tela_por(tx, ty, d->glifo, d->frente, d->fundo);
            else if (m->visto[idx])        /* lembrado: cinza e sem fundo */
                tela_por(tx, ty, d->glifo, CINZA_ESCURO, PRETO);
            else
                tela_por(tx, ty, ' ', CINZA, PRETO);
        }
    }
}
