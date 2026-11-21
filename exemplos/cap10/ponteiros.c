/* ponteiros.c — endereços, *, &, ponteiros e arrays, ponteiros e structs. */
#include <stdio.h>

typedef struct {
    char nome[20];
    int  vida;
} Monstro;

/* Sem ponteiro: a função recebe uma CÓPIA e o original não muda. */
void curar_copia(int vida)
{
    vida += 10;
}

/* Com ponteiro: a função recebe o ENDEREÇO e muda o original. */
void curar(int *vida)
{
    *vida += 10;
}

void trocar(int *a, int *b)
{
    int temporario = *a;
    *a = *b;
    *b = temporario;
}

void ferir(Monstro *m, int dano)
{
    m->vida -= dano;          /* m->vida é o mesmo que (*m).vida */
}

int main(void)
{
    int vida = 5, x = 1, y = 2, i;
    int *p = &vida;                       /* p guarda o endereço de vida */
    int moedas[4] = { 10, 20, 30, 40 };
    int *q = moedas;                      /* um array "vira" ponteiro pro 1º */
    Monstro rato = { "rato", 6 };

    printf("vida = %d, mora no endereço %p\n", vida, (void *)&vida);
    printf("p = %p, e *p = %d\n", (void *)p, *p);
    curar_copia(vida);
    printf("depois de curar_copia: %d\n", vida);
    curar(&vida);
    printf("depois de curar:       %d\n", vida);

    trocar(&x, &y);
    printf("trocados: x = %d, y = %d\n", x, y);

    for (i = 0; i < 4; i++)
        printf("moedas[%d] = %d = *(q + %d) = %d\n", i, moedas[i], i, *(q + i));

    ferir(&rato, 4);
    printf("o %s ficou com %d de vida\n", rato.nome, rato.vida);
    return 0;
}
