/* mochila.c — a "cozinha" do módulo: como as coisas são feitas. */
#include <stdio.h>
#include <string.h>
#include "mochila.h"

static int criadas = 0;   /* static: só este arquivo enxerga */

static int procurar(const Mochila *m, const char *nome)   /* função privada */
{
    int i;
    for (i = 0; i < m->quantidade; i++)
        if (strcmp(m->nomes[i], nome) == 0) return i;
    return -1;
}

void mochila_iniciar(Mochila *m)
{
    m->quantidade = 0;
    criadas++;
}

int mochila_por(Mochila *m, const char *nome)
{
    if (m->quantidade >= MOCHILA_MAX) return 0;
    snprintf(m->nomes[m->quantidade++], sizeof m->nomes[0], "%s", nome);
    return 1;
}

int mochila_tirar(Mochila *m, const char *nome)
{
    int i = procurar(m, nome);
    if (i < 0) return 0;
    m->quantidade--;
    strcpy(m->nomes[i], m->nomes[m->quantidade]);  /* o último tapa o buraco */
    return 1;
}

void mochila_listar(const Mochila *m)
{
    int i;
    printf("Mochila (%d/%d):", m->quantidade, MOCHILA_MAX);
    for (i = 0; i < m->quantidade; i++) printf(" [%s]", m->nomes[i]);
    printf("\n");
}

int mochila_total_criadas(void)
{
    return criadas;
}
