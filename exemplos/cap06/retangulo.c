/* retangulo.c — laços dentro de laços desenham uma sala. */
#include <stdio.h>

int main(void)
{
    int largura = 20, altura = 7, x, y;

    for (y = 0; y < altura; y++) {
        for (x = 0; x < largura; x++) {
            int borda = (y == 0 || y == altura - 1 || x == 0 || x == largura - 1);
            if (x == 9 && y == 3)
                putchar('@');                 /* o herói no meio da sala */
            else if (borda)
                putchar('#');
            else
                putchar('.');
        }
        putchar('\n');                        /* fim da linha */
    }
    return 0;
}
