/*
 * rng.h — gerador de números pseudoaleatórios com semente própria.
 *
 * Por que não usar rand()? Porque cada sistema implementa rand() de um
 * jeito: a mesma semente gera masmorras diferentes no Windows e no
 * Linux. Com o nosso gerador, a semente 1234 gera sempre o mesmo mundo.
 */
#ifndef RNG_H
#define RNG_H

#include <stdint.h>

void     rng_semear(uint32_t semente);
uint32_t rng_proximo(void);                     /* 32 bits "aleatórios" */
int      rng_intervalo(int minimo, int maximo); /* inclui os dois extremos */
int      rng_chance(int porcento);              /* 1 com essa chance    */
uint32_t rng_semente_do_relogio(void);

#endif
