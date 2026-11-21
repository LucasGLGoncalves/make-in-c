/* horda.c — uma lista de monstros que cresce com malloc e realloc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char nome[16];
    int  vida;
} Monstro;

typedef struct {
    Monstro *itens;       /* aponta para um bloco no heap  */
    int      quantidade;  /* quantos estão em uso          */
    int      capacidade;  /* quantos cabem antes de crescer */
} Horda;

int horda_adicionar(Horda *h, const char *nome, int vida)
{
    if (h->quantidade == h->capacidade) {
        int nova = h->capacidade ? h->capacidade * 2 : 4;
        Monstro *maior = realloc(h->itens, (size_t)nova * sizeof *maior);
        if (maior == NULL) return 0;          /* sem memória: nada muda */
        h->itens = maior;
        h->capacidade = nova;
        printf("  (a horda cresceu para %d lugares)\n", nova);
    }
    snprintf(h->itens[h->quantidade].nome, sizeof h->itens[0].nome, "%s", nome);
    h->itens[h->quantidade].vida = vida;
    h->quantidade++;
    return 1;
}

/* Tira os mortos, mantendo a ordem dos vivos. */
void horda_limpar(Horda *h)
{
    int i, vivos = 0;
    for (i = 0; i < h->quantidade; i++)
        if (h->itens[i].vida > 0)
            h->itens[vivos++] = h->itens[i];
    h->quantidade = vivos;
}

void horda_liberar(Horda *h)
{
    free(h->itens);
    h->itens = NULL;
    h->quantidade = h->capacidade = 0;
}

int main(void)
{
    static const char *NOMES[] = { "rato", "goblin", "morcego", "kobold", "orc" };
    Horda h = { NULL, 0, 0 };
    int i;

    for (i = 0; i < 11; i++) {
        printf("Chega um %s.\n", NOMES[i % 5]);
        horda_adicionar(&h, NOMES[i % 5], 1 + i % 3);
    }
    for (i = 0; i < h.quantidade; i++)           /* uma bola de fogo! */
        h.itens[i].vida -= 2;
    horda_limpar(&h);
    printf("Depois da bola de fogo sobraram %d:", h.quantidade);
    for (i = 0; i < h.quantidade; i++) printf(" %s", h.itens[i].nome);
    printf("\n");
    horda_liberar(&h);                           /* devolve a memória */
    return 0;
}
