/*
 * rng.c — gerador "xorshift32" (George Marsaglia, 2003).
 */
#include <time.h>
#include "rng.h"

static uint32_t estado = 2463534242u;   /* nunca pode ser zero */

void rng_semear(uint32_t semente)
{
    estado = semente ? semente : 2463534242u;
}

uint32_t rng_proximo(void)
{
    uint32_t x = estado;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    estado = x;
    return x;
}

int rng_intervalo(int minimo, int maximo)
{
    uint32_t largura;
    if (maximo <= minimo) return minimo;
    largura = (uint32_t)(maximo - minimo) + 1u;
    return minimo + (int)(rng_proximo() % largura);
}

int rng_chance(int porcento)
{
    return rng_intervalo(1, 100) <= porcento;
}

uint32_t rng_semente_do_relogio(void)
{
    return (uint32_t)time(NULL) ^ ((uint32_t)clock() << 16);
}
