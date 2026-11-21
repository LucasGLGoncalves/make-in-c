/*
 * dados_jogo.c — lê itens.txt e monstros.txt para as tabelas do jogo.
 *
 * Os números do jogo ficam em arquivos de texto: para deixar o orc mais
 * forte não é preciso recompilar nada, basta editar dados/monstros.txt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "dados.h"
#include "coroa.h"

ItemDef    ITENS[MAX_ITEM_DEFS];
int        N_ITENS = 0;
MonstroDef MONSTROS[MAX_MONSTRO_DEFS];
int        N_MONSTROS = 0;

/* Procura 'texto' numa lista de nomes; devolve a posição ou -1. */
static int indice_de(const char *texto, const char *const nomes[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        if (strcmp(texto, nomes[i]) == 0) return i;
    return -1;
}

static int cor_por_nome(const char *nome)
{
    static const char *const CORES[16] = {
        "PRETO", "AZUL", "VERDE", "CIANO", "VERMELHO", "MAGENTA", "MARROM",
        "CINZA", "CINZA_ESCURO", "AZUL_CLARO", "VERDE_CLARO", "CIANO_CLARO",
        "VERMELHO_CLARO", "MAGENTA_CLARO", "AMARELO", "BRANCO"
    };
    int i = indice_de(nome, CORES, 16);
    return i >= 0 ? i : CINZA;
}

/* "$" vira '$'; "U+25A0" vira o código Unicode 0x25A0. */
static unsigned int glifo_por_texto(const char *texto)
{
    if (texto[0] == 'U' && texto[1] == '+')
        return (unsigned int)strtoul(texto + 2, NULL, 16);
    return (unsigned char)texto[0];
}

static int carregar_itens(const char *arquivo)
{
    static const char *const TIPOS[] = { "ouro", "arma", "armadura", "pocao",
                                         "pergaminho", "coroa" };
    static const char *const SLOTS[] = { "nenhum", "mao", "escudo", "corpo",
                                         "cabeca", "anel" };
    char linha[512], *c[12];
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;

    while (N_ITENS < MAX_ITEM_DEFS && dados_ler_linha(f, linha, sizeof linha)) {
        ItemDef *d = &ITENS[N_ITENS];
        if (dados_separar(linha, '|', c, 12) != 12) continue;  /* linha torta */
        snprintf(d->id, sizeof d->id, "%s", c[0]);
        snprintf(d->nome, sizeof d->nome, "%s", c[1]);
        d->glifo    = glifo_por_texto(c[2]);
        d->cor      = cor_por_nome(c[3]);
        d->tipo     = indice_de(c[4], TIPOS, 6);
        d->slot     = indice_de(c[5], SLOTS, 6);
        d->valor    = atoi(c[6]);
        d->ataque   = atoi(c[7]);
        d->defesa   = atoi(c[8]);
        d->efeito   = atoi(c[9]);
        d->prof_min = atoi(c[10]);
        snprintf(d->descricao, sizeof d->descricao, "%s", c[11]);
        if (d->tipo < 0 || d->slot < 0) continue;              /* nome errado */
        N_ITENS++;
    }
    fclose(f);
    return N_ITENS > 0;
}

static int carregar_monstros(const char *arquivo)
{
    char linha[512], *c[11];
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;

    while (N_MONSTROS < MAX_MONSTRO_DEFS && dados_ler_linha(f, linha, sizeof linha)) {
        MonstroDef *m = &MONSTROS[N_MONSTROS];
        if (dados_separar(linha, '|', c, 11) != 11) continue;
        snprintf(m->id, sizeof m->id, "%s", c[0]);
        snprintf(m->nome, sizeof m->nome, "%s", c[1]);
        m->glifo    = glifo_por_texto(c[2]);
        m->cor      = cor_por_nome(c[3]);
        m->hp       = atoi(c[4]);
        m->ataque   = atoi(c[5]);
        m->defesa   = atoi(c[6]);
        m->xp       = atoi(c[7]);
        m->prof_min = atoi(c[8]);
        m->prof_max = atoi(c[9]);
        m->erratico = atoi(c[10]);
        N_MONSTROS++;
    }
    fclose(f);
    return N_MONSTROS > 0;
}

int dados_carregar_tudo(void)
{
    return carregar_itens("dados/itens.txt") &&
           carregar_monstros("dados/monstros.txt");
}

int item_por_id(const char *id)
{
    int i;
    for (i = 0; i < N_ITENS; i++)
        if (strcmp(ITENS[i].id, id) == 0) return i;
    return -1;
}
