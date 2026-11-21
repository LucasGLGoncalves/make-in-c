/* mapa.c — um mapa em array 2D e um herói que anda por ele. */
#include <stdio.h>

#define LARGURA 20
#define ALTURA  8

int main(void)
{
    char mapa[ALTURA][LARGURA + 1] = {     /* +1: espaço para o '\0' */
        "####################",
        "#..........#.......#",
        "#..........#...$...#",
        "#....~~~...#.......#",
        "#....~~~...........#",
        "#..........#.......#",
        "#..........#.......#",
        "####################"
    };
    int x = 2, y = 2, ouro = 0;
    char tecla;

    for (;;) {
        int i, j, nx = x, ny = y;
        for (j = 0; j < ALTURA; j++) {             /* desenha o mapa */
            for (i = 0; i < LARGURA; i++)
                putchar((i == x && j == y) ? '@' : mapa[j][i]);
            putchar('\n');
        }
        printf("Ouro: %d   (w/a/s/d + Enter para andar, q para sair) ", ouro);
        if (scanf(" %c", &tecla) != 1 || tecla == 'q') break;

        if (tecla == 'w') ny--;
        if (tecla == 's') ny++;
        if (tecla == 'a') nx--;
        if (tecla == 'd') nx++;

        if (mapa[ny][nx] == '#') {
            printf("\nUma parede.\n");
        } else if (mapa[ny][nx] == '~') {
            printf("\nÁgua funda. Melhor não.\n");
        } else {
            x = nx;
            y = ny;
            if (mapa[y][x] == '$') {
                ouro += 50;
                mapa[y][x] = '.';                  /* o ouro sai do mapa */
                printf("\nVocê achou 50 moedas!\n");
            }
        }
    }
    return 0;
}
