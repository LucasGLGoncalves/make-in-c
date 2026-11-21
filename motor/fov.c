/*
 * fov.c — algoritmo de Bresenham e campo de visão por raios.
 */
#include <stdlib.h>
#include <string.h>
#include "fov.h"

void linha_percorrer(int x0, int y0, int x1, int y1,
                     LinhaPasso passo, void *ctx)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int erro = dx + dy, e2;

    for (;;) {
        if (!passo(x0, y0, ctx)) return;
        if (x0 == x1 && y0 == y1) return;
        e2 = 2 * erro;
        if (e2 >= dy) { erro += dy; x0 += sx; }
        if (e2 <= dx) { erro += dx; y0 += sy; }
    }
}

typedef struct {
    const Mapa *mapa;
    int x0, y0, x1, y1;
    int livre;
} Visada;

static int passo_visada(int x, int y, void *ctx)
{
    Visada *v = ctx;
    if ((x == v->x0 && y == v->y0) || (x == v->x1 && y == v->y1))
        return 1;                       /* as pontas não contam */
    if (mapa_opaco(v->mapa, x, y)) {
        v->livre = 0;
        return 0;
    }
    return 1;
}

int linha_de_visao(const Mapa *m, int x0, int y0, int x1, int y1)
{
    Visada v;
    v.mapa = m;
    v.x0 = x0; v.y0 = y0; v.x1 = x1; v.y1 = y1;
    v.livre = 1;
    linha_percorrer(x0, y0, x1, y1, passo_visada, &v);
    return v.livre;
}

typedef struct {
    Mapa *mapa;
    int ox, oy, raio2;
} Raio;

static int passo_raio(int x, int y, void *ctx)
{
    Raio *r = ctx;
    int dx = x - r->ox, dy = y - r->oy;
    if (!mapa_dentro(r->mapa, x, y)) return 0;
    if (dx * dx + dy * dy > r->raio2) return 0;      /* longe demais */
    r->mapa->visivel[y * r->mapa->largura + x] = 1;
    if (x == r->ox && y == r->oy) return 1;          /* a origem não bloqueia */
    return !mapa_opaco(r->mapa, x, y);               /* parede: o raio para */
}

void fov_calcular(Mapa *m, int ox, int oy, int raio)
{
    Raio r;
    int i, x, y, n = m->largura * m->altura;

    memset(m->visivel, 0, (size_t)n);
    r.mapa = m; r.ox = ox; r.oy = oy;
    r.raio2 = raio * raio + raio;          /* "+ raio" deixa o círculo redondo */

    /* lança um raio para cada ponto da borda do quadrado ao redor */
    for (i = -raio; i <= raio; i++) {
        linha_percorrer(ox, oy, ox + i, oy - raio, passo_raio, &r);
        linha_percorrer(ox, oy, ox + i, oy + raio, passo_raio, &r);
        linha_percorrer(ox, oy, ox - raio, oy + i, passo_raio, &r);
        linha_percorrer(ox, oy, ox + raio, oy + i, passo_raio, &r);
    }

    /* acabamento: paredes vizinhas de chão visível também aparecem */
    for (y = 1; y < m->altura - 1; y++) {
        for (x = 1; x < m->largura - 1; x++) {
            int idx = y * m->largura + x, dx, dy;
            if (m->visivel[idx] != 1 || mapa_opaco(m, x, y)) continue;
            for (dy = -1; dy <= 1; dy++)
                for (dx = -1; dx <= 1; dx++) {
                    int viz = (y + dy) * m->largura + (x + dx);
                    if (!m->visivel[viz] && mapa_opaco(m, x + dx, y + dy))
                        m->visivel[viz] = 2;   /* 2 = parede revelada */
                }
        }
    }

    for (i = 0; i < n; i++)
        if (m->visivel[i]) {
            m->visivel[i] = 1;
            m->visto[i] = 1;
        }
}

void fov_revelar_tudo(Mapa *m)
{
    size_t n = (size_t)m->largura * (size_t)m->altura;
    memset(m->visivel, 1, n);
    memset(m->visto, 1, n);
}
