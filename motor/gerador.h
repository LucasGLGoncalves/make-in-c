/*
 * gerador.h — geração procedural de masmorras e cavernas.
 */
#ifndef GERADOR_H
#define GERADOR_H

#include "mapa.h"

typedef struct { int x, y, l, a; } Sala;

int  gerar_masmorra(Mapa *m, int parede, int chao, Sala *salas, int max_salas);
void gerar_caverna(Mapa *m, int parede, int chao, int porcento, int passos);
int  mapa_inundar(const Mapa *m, int x, int y, unsigned char *marcas);
void sala_centro(const Sala *s, int *x, int *y);
int  sala_ponto_livre(const Mapa *m, const Sala *s, int *x, int *y);

#endif
