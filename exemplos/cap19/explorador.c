/* explorador.c — um mapa maior que a tela e uma câmera que segue o herói. */
#include <stdio.h>
#include "term.h"
#include "tela.h"
#include "mapa.h"

enum { MAR, AREIA, GRAMA, ARVORE, MONTANHA, ESTRADA, PAREDE, PISO, PORTA };

static const TileDef TILES[] = {
    { G_APROX,        AZUL_CLARO, AZUL,  T_SOLIDO,           "mar"      },
    { G_SOMBRA_LEVE,  AMARELO,    PRETO, 0,                  "areia"    },
    { '.',            VERDE,      PRETO, 0,                  "grama"    },
    { G_PAUS,         VERDE_CLARO,PRETO, T_SOLIDO | T_OPACO, "árvore"   },
    { G_TRI_CIMA,     CINZA,      PRETO, T_SOLIDO | T_OPACO, "montanha" },
    { G_SOMBRA_LEVE,  MARROM,     PRETO, 0,                  "estrada"  },
    { G_BLOCO,        MARROM,     PRETO, T_SOLIDO | T_OPACO, "parede"   },
    { G_PONTO,        MARROM,     PRETO, 0,                  "piso"     },
    { '+',            AMARELO,    PRETO, 0,                  "porta"    }
};
static const char LEGENDA[] = "~,.T^:H=+";

static int heroi_x, heroi_y;

static int achar_heroi(void *ctx, char c, int x, int y)
{
    (void)ctx;
    if (c == '@') { heroi_x = x; heroi_y = y; }
    return ESTRADA;
}

int main(void)
{
    Mapa mapa = { 0 };
    Camera cam = { 0, 0, 1, 1, 60, 21 };      /* janela de 60x21 na posição (1,1) */
    int tecla = 0;

    if (!mapa_carregar(&mapa, "ilha.txt", TILES, LEGENDA, achar_heroi, NULL)) {
        printf("Não achei ilha.txt\n");
        return 1;
    }
    term_iniciar();
    while (tecla != 'q') {
        int nx = heroi_x, ny = heroi_y, tx, ty;
        camera_centralizar(&cam, &mapa, heroi_x, heroi_y);
        tela_limpar();
        tela_moldura(0, 0, 62, 23, 1, CINZA, PRETO);
        mapa_desenhar(&mapa, &cam, 0);
        if (camera_na_tela(&cam, heroi_x, heroi_y, &tx, &ty))
            tela_por(tx, ty, '@', BRANCO, PRETO);
        tela_textof(64, 2, AMARELO, PRETO, "Posição %d,%d", heroi_x, heroi_y);
        tela_textof(64, 3, CINZA, PRETO, "Câmera  %d,%d", cam.x, cam.y);
        tela_textof(64, 5, CINZA, PRETO, "Pisando em:");
        tela_texto(64, 6, BRANCO, PRETO,
                   TILES[mapa_tile(&mapa, heroi_x, heroi_y)].nome);
        tela_texto(64, 21, CINZA_ESCURO, PRETO, "setas: andar");
        tela_texto(64, 22, CINZA_ESCURO, PRETO, "q: sair");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == TECLA_CIMA) ny--;
        if (tecla == TECLA_BAIXO) ny++;
        if (tecla == TECLA_ESQUERDA) nx--;
        if (tecla == TECLA_DIREITA) nx++;
        if (!mapa_bloqueia(&mapa, nx, ny)) { heroi_x = nx; heroi_y = ny; }
    }
    term_encerrar();
    mapa_destruir(&mapa);
    return 0;
}
