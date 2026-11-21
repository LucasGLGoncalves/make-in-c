/*
 * gerador.c — salas e corredores, autômato celular e "inundação".
 */
#include <stdlib.h>
#include <string.h>
#include "rng.h"
#include "gerador.h"

void sala_centro(const Sala *s, int *x, int *y)
{
    *x = s->x + s->l / 2;
    *y = s->y + s->a / 2;
}

static int salas_encostam(const Sala *a, const Sala *b)
{
    /* a margem de 1 garante pelo menos uma parede entre as salas */
    return a->x - 1 <= b->x + b->l && b->x - 1 <= a->x + a->l &&
           a->y - 1 <= b->y + b->a && b->y - 1 <= a->y + a->a;
}

static void cavar_sala(Mapa *m, const Sala *s, int chao)
{
    int x, y;
    for (y = s->y; y < s->y + s->a; y++)
        for (x = s->x; x < s->x + s->l; x++)
            mapa_por(m, x, y, chao);
}

static void cavar_corredor(Mapa *m, int x0, int y0, int x1, int y1, int chao)
{
    int x = x0, y = y0;
    int horizontal_primeiro = rng_chance(50);
    while (x != x1 || y != y1) {
        mapa_por(m, x, y, chao);
        if ((horizontal_primeiro && x != x1) || y == y1)
            x += (x1 > x) ? 1 : -1;
        else
            y += (y1 > y) ? 1 : -1;
    }
    mapa_por(m, x1, y1, chao);
}

int gerar_masmorra(Mapa *m, int parede, int chao, Sala *salas, int max_salas)
{
    int n = 0, tentativa, i;
    mapa_preencher(m, parede);
    for (tentativa = 0; tentativa < max_salas * 10 && n < max_salas; tentativa++) {
        Sala s;
        int encostou = 0;
        s.l = rng_intervalo(4, 12);
        s.a = rng_intervalo(3, 7);
        s.x = rng_intervalo(1, m->largura - s.l - 2);
        s.y = rng_intervalo(1, m->altura - s.a - 2);
        for (i = 0; i < n && !encostou; i++)
            encostou = salas_encostam(&s, &salas[i]);
        if (encostou) continue;

        cavar_sala(m, &s, chao);
        if (n > 0) {                     /* liga à sala anterior */
            int ax, ay, bx, by;
            sala_centro(&salas[n - 1], &ax, &ay);
            sala_centro(&s, &bx, &by);
            cavar_corredor(m, ax, ay, bx, by, chao);
        }
        salas[n++] = s;
    }
    return n;
}

int sala_ponto_livre(const Mapa *m, const Sala *s, int *x, int *y)
{
    int tentativa;
    for (tentativa = 0; tentativa < 30; tentativa++) {
        *x = rng_intervalo(s->x, s->x + s->l - 1);
        *y = rng_intervalo(s->y, s->y + s->a - 1);
        if (!mapa_bloqueia(m, *x, *y)) return 1;
    }
    return 0;
}

int mapa_inundar(const Mapa *m, int x, int y, unsigned char *marcas)
{
    int n = m->largura * m->altura, inicio = 0, fim = 0, total = 0;
    int *fila;
    if (!mapa_dentro(m, x, y) || mapa_bloqueia(m, x, y)) return 0;
    fila = malloc((size_t)n * sizeof *fila);
    if (fila == NULL) return 0;
    fila[fim++] = y * m->largura + x;
    marcas[y * m->largura + x] = 1;
    while (inicio < fim) {
        int atual = fila[inicio++], d;
        static const int DX[4] = { 1, -1, 0, 0 }, DY[4] = { 0, 0, 1, -1 };
        int cx = atual % m->largura, cy = atual / m->largura;
        total++;
        for (d = 0; d < 4; d++) {
            int nx = cx + DX[d], ny = cy + DY[d], viz;
            if (!mapa_dentro(m, nx, ny) || mapa_bloqueia(m, nx, ny)) continue;
            viz = ny * m->largura + nx;
            if (marcas[viz]) continue;
            marcas[viz] = 1;
            fila[fim++] = viz;
        }
    }
    free(fila);
    return total;
}

static int paredes_em_volta(const Mapa *m, int x, int y, int parede)
{
    int dx, dy, total = 0;
    for (dy = -1; dy <= 1; dy++)
        for (dx = -1; dx <= 1; dx++) {
            int t = mapa_tile(m, x + dx, y + dy);
            if (t < 0 || t == parede) total++;   /* fora conta como parede */
        }
    return total;
}

void gerar_caverna(Mapa *m, int parede, int chao, int porcento, int passos)
{
    int x, y, p, n = m->largura * m->altura;
    unsigned char *copia = malloc((size_t)n);
    unsigned char *marcas = calloc((size_t)n, 1);
    int maior = 0, maior_x = 0, maior_y = 0;
    if (copia == NULL || marcas == NULL) { free(copia); free(marcas); return; }

    /* 1. ruído: cada célula é parede com uma certa chance */
    for (y = 0; y < m->altura; y++)
        for (x = 0; x < m->largura; x++) {
            int borda = x == 0 || y == 0 || x == m->largura - 1 || y == m->altura - 1;
            mapa_por(m, x, y, (borda || rng_chance(porcento)) ? parede : chao);
        }

    /* 2. suavização: cada célula imita a maioria das vizinhas */
    for (p = 0; p < passos; p++) {
        for (y = 1; y < m->altura - 1; y++)
            for (x = 1; x < m->largura - 1; x++)
                copia[y * m->largura + x] = (unsigned char)
                    (paredes_em_volta(m, x, y, parede) >= 5 ? parede : chao);
        for (y = 1; y < m->altura - 1; y++)
            for (x = 1; x < m->largura - 1; x++)
                mapa_por(m, x, y, copia[y * m->largura + x]);
    }

    /* 3. fica só a maior região conectada; as outras viram rocha */
    for (y = 0; y < m->altura; y++)
        for (x = 0; x < m->largura; x++)
            if (!marcas[y * m->largura + x] && !mapa_bloqueia(m, x, y)) {
                int tamanho = mapa_inundar(m, x, y, marcas);
                if (tamanho > maior) { maior = tamanho; maior_x = x; maior_y = y; }
            }
    memset(marcas, 0, (size_t)n);
    if (maior > 0) mapa_inundar(m, maior_x, maior_y, marcas);
    for (y = 0; y < m->altura; y++)
        for (x = 0; x < m->largura; x++)
            if (!marcas[y * m->largura + x]) mapa_por(m, x, y, parede);

    free(copia);
    free(marcas);
}
