/* caminhos.c — o mapa de distâncias e monstros que o descem. */
#include <stdlib.h>
#include "term.h"
#include "tela.h"
#include "rng.h"
#include "mapa.h"
#include "gerador.h"
#include "caminho.h"

static const TileDef TILES[] = {
    { G_SOMBRA_MEDIA, CINZA_ESCURO, PRETO, T_SOLIDO | T_OPACO, "parede" },
    { ' ',            CINZA,        PRETO, 0,                  "chão"   }
};

int main(void)
{
    static const int CORES[5] = { BRANCO, AMARELO, VERDE_CLARO, CIANO, AZUL };
    Mapa m = { 0 };
    Sala salas[12];
    int *dist, x, y, mx[3], my[3], i, tecla = 0, mostrar = 1;

    rng_semear(rng_semente_do_relogio());
    mapa_criar(&m, 80, 22, TILES);
    gerar_masmorra(&m, 0, 1, salas, 12);
    dist = malloc(sizeof(int) * 80 * 22);
    sala_centro(&salas[0], &x, &y);
    for (i = 0; i < 3; i++) sala_centro(&salas[(i + 2) % 12], &mx[i], &my[i]);

    term_iniciar();
    while (tecla != 'q') {
        int nx = x, ny = y, j;
        caminho_distancias(&m, dist, x, y, 0, 0);
        tela_limpar();
        for (j = 0; j < 22; j++)
            for (i = 0; i < 80; i++) {
                int d = dist[j * 80 + i];
                if (mapa_bloqueia(&m, i, j))
                    tela_por(i, j, G_SOMBRA_MEDIA, CINZA_ESCURO, PRETO);
                else if (mostrar && d < CAMINHO_LONGE)  /* o último dígito da distância */
                    tela_por(i, j, '0' + d % 10, CORES[(d / 10) % 5], PRETO);
            }
        for (i = 0; i < 3; i++) tela_por(mx[i], my[i], 'M', VERMELHO_CLARO, PRETO);
        tela_por(x, y, '@', BRANCO, AZUL);
        tela_texto(0, 23, CINZA, PRETO, "setas: andar   d: mostrar/esconder distâncias   q: sair");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == 'd') mostrar = !mostrar;
        if (tecla == TECLA_CIMA) ny--;
        if (tecla == TECLA_BAIXO) ny++;
        if (tecla == TECLA_ESQUERDA) nx--;
        if (tecla == TECLA_DIREITA) nx++;
        if (!mapa_bloqueia(&m, nx, ny)) { x = nx; y = ny; }
        caminho_distancias(&m, dist, x, y, 0, 0);
        for (i = 0; i < 3; i++) {                 /* cada monstro desce um degrau */
            int dx, dy;
            if (caminho_descer(&m, dist, mx[i], my[i], 0, &dx, &dy) &&
                !(mx[i] + dx == x && my[i] + dy == y)) {
                mx[i] += dx;
                my[i] += dy;
            }
        }
    }
    term_encerrar();
    free(dist);
    mapa_destruir(&m);
    return 0;
}
