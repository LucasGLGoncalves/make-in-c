/*
 * fov.h — linhas, linha de visão e campo de visão ("field of view").
 */
#ifndef FOV_H
#define FOV_H

#include "mapa.h"

/* Função chamada para cada ponto da linha. Devolve 0 para parar. */
typedef int (*LinhaPasso)(int x, int y, void *ctx);

void linha_percorrer(int x0, int y0, int x1, int y1,
                     LinhaPasso passo, void *ctx);
int  linha_de_visao(const Mapa *m, int x0, int y0, int x1, int y1);
void fov_calcular(Mapa *m, int ox, int oy, int raio);
void fov_revelar_tudo(Mapa *m);

#endif
