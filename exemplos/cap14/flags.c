/* flags.c — várias informações sim/não guardadas nos bits de um número. */
#include <stdio.h>

#define SOLIDO    (1u << 0)   /* 0000 0001 */
#define OPACO     (1u << 1)   /* 0000 0010 */
#define MOLHADO   (1u << 2)   /* 0000 0100 */
#define QUEIMA    (1u << 3)   /* 0000 1000 */

void mostrar_bits(const char *nome, unsigned int v)
{
    int b;
    printf("%-8s ", nome);
    for (b = 7; b >= 0; b--) putchar((v >> b) & 1u ? '1' : '0');
    printf("  (%u)\n", v);
}

int main(void)
{
    unsigned int parede = SOLIDO | OPACO;        /* OU junta bits     */
    unsigned int lava = QUEIMA;
    unsigned int pantano = MOLHADO;

    mostrar_bits("parede", parede);
    mostrar_bits("lava", lava);
    mostrar_bits("pantano", pantano);

    if (parede & SOLIDO) printf("A parede é sólida.\n");   /* E testa um bit */
    if (!(lava & SOLIDO)) printf("Dá para pisar na lava (e se queimar).\n");

    pantano |= OPACO;                 /* liga um bit: neblina no pântano */
    mostrar_bits("pantano", pantano);
    pantano &= ~MOLHADO;              /* desliga um bit: o pântano secou */
    mostrar_bits("pantano", pantano);
    pantano ^= QUEIMA;                /* inverte um bit: pegou fogo!     */
    mostrar_bits("pantano", pantano);
    return 0;
}
