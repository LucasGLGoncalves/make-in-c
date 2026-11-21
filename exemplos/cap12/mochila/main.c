/* main.c — usa o módulo mochila sem saber como ele funciona por dentro. */
#include <stdio.h>
#include "mochila.h"

int main(void)
{
    Mochila m;
    mochila_iniciar(&m);
    mochila_por(&m, "corda");
    mochila_por(&m, "tocha");
    mochila_por(&m, "pão");
    mochila_listar(&m);
    if (mochila_tirar(&m, "tocha")) printf("Você acende e larga a tocha.\n");
    mochila_listar(&m);
    printf("Mochilas criadas: %d\n", mochila_total_criadas());
    return 0;
}
