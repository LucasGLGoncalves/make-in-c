/*
 * caminho.c — busca em largura (BFS) a partir do alvo.
 *
 * Em vez de calcular "o caminho do monstro até o jogador", calculamos
 * a distância de TODAS as células até o jogador. Depois, qualquer
 * monstro só precisa dar um passo para a vizinha mais próxima do alvo.
 */
#include <stdlib.h>
#include "caminho.h"

static const int DX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
static const int DY[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

void caminho_distancias(const Mapa *m, int *dist, int alvo_x, int alvo_y,
                        int diagonais, int limite)
{
    int n = m->largura * m->altura;
    int *fila = malloc((size_t)n * sizeof *fila);
    int inicio = 0, fim = 0, i, d;
    int direcoes = diagonais ? 8 : 4;

    for (i = 0; i < n; i++) dist[i] = CAMINHO_LONGE;
    if (fila == NULL || !mapa_dentro(m, alvo_x, alvo_y)) {
        free(fila);
        return;
    }

    dist[alvo_y * m->largura + alvo_x] = 0;
    fila[fim++] = alvo_y * m->largura + alvo_x;

    while (inicio < fim) {
        int atual = fila[inicio++];
        int x = atual % m->largura, y = atual / m->largura;
        if (limite > 0 && dist[atual] >= limite) continue;
        for (d = 0; d < direcoes; d++) {
            int nx = x + DX[d], ny = y + DY[d], viz;
            if (!mapa_dentro(m, nx, ny) || mapa_bloqueia(m, nx, ny)) continue;
            viz = ny * m->largura + nx;
            if (dist[viz] != CAMINHO_LONGE) continue;  /* já visitada */
            dist[viz] = dist[atual] + 1;
            fila[fim++] = viz;
        }
    }
    free(fila);
}

/* Escolhe a vizinha de (x,y) com a menor distância. Devolve 0 se
 * nenhuma vizinha está mais perto do alvo do que (x,y). */
int caminho_descer(const Mapa *m, const int *dist, int x, int y,
                   int diagonais, int *dx, int *dy)
{
    int melhor = dist[y * m->largura + x], achou = 0, d;
    int direcoes = diagonais ? 8 : 4;
    for (d = 0; d < direcoes; d++) {
        int nx = x + DX[d], ny = y + DY[d];
        if (!mapa_dentro(m, nx, ny)) continue;
        if (dist[ny * m->largura + nx] < melhor) {
            melhor = dist[ny * m->largura + nx];
            *dx = DX[d];
            *dy = DY[d];
            achou = 1;
        }
    }
    return achou;
}
