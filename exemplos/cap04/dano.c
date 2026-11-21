/* dano.c — expressões, divisão inteira, resto e conversões. */
#include <stdio.h>

int main(void)
{
    int ataque = 17, defesa = 5;
    int dano = ataque - defesa;               /* 12 */
    int golpes = 100 / dano;                  /* divisão inteira: 8 */
    int sobra = 100 % dano;                   /* resto: 4 */
    double media = (double)ataque / 2;        /* 8.5 (com o cast) */
    double errado = ataque / 2;               /* 8.0 (sem o cast!) */
    int xp = 0;

    printf("Dano por golpe: %d\n", dano);
    printf("Golpes para tirar 100 de vida: %d (sobram %d)\n", golpes, sobra);
    printf("Metade do ataque: %.1f (certo) e %.1f (errado)\n", media, errado);

    xp += 15;          /* o mesmo que xp = xp + 15 */
    xp *= 2;           /* xp = xp * 2  -> 30 */
    xp++;              /* soma 1       -> 31 */
    printf("XP: %d\n", xp);

    printf("Precedência: 2 + 3 * 4 = %d, mas (2 + 3) * 4 = %d\n",
           2 + 3 * 4, (2 + 3) * 4);
    printf("Par ou ímpar? 7 %% 2 = %d\n", 7 % 2);
    return 0;
}
