/*
 * caminho.h — mapas de distância (busca em largura) para que
 * monstros encontrem o caminho até o jogador.
 */
#ifndef CAMINHO_H
#define CAMINHO_H

#include "mapa.h"

#define CAMINHO_LONGE 30000   /* "inalcançável" */

void caminho_distancias(const Mapa *m, int *dist, int alvo_x, int alvo_y,
                        int diagonais, int limite);
int  caminho_descer(const Mapa *m, const int *dist, int x, int y,
                    int diagonais, int *dx, int *dy);

#endif
