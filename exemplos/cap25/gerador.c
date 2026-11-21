/* gerador.c — masmorras e cavernas geradas a partir de uma semente. */
#include "term.h"
#include "tela.h"
#include "rng.h"
#include "mapa.h"
#include "gerador.h"

static const TileDef TILES[] = {
    { G_SOMBRA_MEDIA, MARROM, PRETO, T_SOLIDO | T_OPACO, "rocha" },
    { G_PONTO,        CINZA,  PRETO, 0,                  "chão"  }
};

int main(void)
{
    Mapa m = { 0 };
    Sala salas[14];
    Camera cam = { 0, 0, 0, 0, 80, 22 };
    uint32_t semente = 1;
    int tecla = 'm', tipo = 'm', n = 0, i;

    mapa_criar(&m, 80, 22, TILES);
    term_iniciar();
    while (tecla != 'q') {
        if (tecla == 'm' || tecla == 'c') tipo = tecla;
        if (tecla == 'n') semente++;
        if (tecla == 'r') semente = rng_semente_do_relogio();
        rng_semear(semente);                       /* mesma semente = mesmo mapa */
        if (tipo == 'm') n = gerar_masmorra(&m, 0, 1, salas, 14);
        else             gerar_caverna(&m, 0, 1, 45, 4);

        tela_limpar();
        mapa_desenhar(&m, &cam, 0);
        if (tipo == 'm')
            for (i = 0; i < n; i++)                /* numera as salas */
                tela_textof(salas[i].x, salas[i].y, AMARELO, PRETO, "%d", i);
        tela_textof(0, 23, CINZA, PRETO,
                    "semente %u   m masmorra  c caverna  n próxima  r aleatória  q sai",
                    (unsigned int)semente);
        tela_mostrar();
        tecla = term_esperar_tecla();
    }
    term_encerrar();
    mapa_destruir(&m);
    return 0;
}
