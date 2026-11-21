/* visao.c — campo de visão: o que o herói vê agora e o que lembra. */
#include "term.h"
#include "tela.h"
#include "rng.h"
#include "mapa.h"
#include "fov.h"
#include "gerador.h"

static const TileDef TILES[] = {
    { G_SOMBRA_MEDIA, CINZA,        PRETO, T_SOLIDO | T_OPACO, "parede" },
    { G_PONTO,        CINZA,        PRETO, 0,                  "chão"   }
};

int main(void)
{
    Mapa m = { 0 };
    Sala salas[12];
    Camera cam = { 0, 0, 0, 0, 80, 22 };
    int x, y, raio = 7, tecla = 0, usar_fov = 1;

    rng_semear(rng_semente_do_relogio());
    mapa_criar(&m, 80, 22, TILES);
    gerar_masmorra(&m, 0, 1, salas, 12);
    sala_centro(&salas[0], &x, &y);
    term_iniciar();
    while (tecla != 'q') {
        int nx = x, ny = y;
        fov_calcular(&m, x, y, raio);
        tela_limpar();
        mapa_desenhar(&m, &cam, usar_fov);
        tela_por(x, y, '@', AMARELO, PRETO);
        tela_textof(0, 23, CINZA, PRETO,
                    "raio %d (+/-)   f: visão %s   setas: andar   q: sair",
                    raio, usar_fov ? "ligada " : "desligada");
        tela_mostrar();
        tecla = term_esperar_tecla();
        if (tecla == TECLA_CIMA) ny--;
        if (tecla == TECLA_BAIXO) ny++;
        if (tecla == TECLA_ESQUERDA) nx--;
        if (tecla == TECLA_DIREITA) nx++;
        if (tecla == '+' && raio < 30) raio++;
        if (tecla == '-' && raio > 1) raio--;
        if (tecla == 'f') usar_fov = !usar_fov;
        if (!mapa_bloqueia(&m, nx, ny)) { x = nx; y = ny; }
    }
    term_encerrar();
    mapa_destruir(&m);
    return 0;
}
