/* dados.c — funções e números aleatórios. */
#include <stdio.h>
#include <stdlib.h>   /* rand, srand */
#include <time.h>     /* time */

/* Rola um dado de 'lados' faces: devolve de 1 até 'lados'. */
int rolar(int lados)
{
    return rand() % lados + 1;
}

/* Rola 'quantos' dados e soma tudo. */
int rolar_varios(int quantos, int lados)
{
    int total = 0, i;
    for (i = 0; i < quantos; i++)
        total += rolar(lados);
    return total;
}

int main(void)
{
    int i;
    srand((unsigned int)time(NULL));   /* semente diferente a cada execução */
    printf("Um d6:  %d\n", rolar(6));
    printf("Um d20: %d\n", rolar(20));
    printf("3d6:    %d\n", rolar_varios(3, 6));
    printf("Dez rolagens de d6:");
    for (i = 0; i < 10; i++)
        printf(" %d", rolar(6));
    printf("\n");
    return 0;
}
