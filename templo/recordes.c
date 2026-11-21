/*
 * recordes.c — a tabela dos cinco melhores, guardada em "recordes.txt".
 */
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "templo.h"

#define MAX_RECORDES 5
#define ARQUIVO_RECORDES "recordes.txt"

typedef struct {
    long pontos;
    char nome[24];
} Recorde;

static int recordes_ler(Recorde r[])
{
    int n = 0;
    FILE *f = fopen(ARQUIVO_RECORDES, "r");
    if (f == NULL) return 0;                 /* ainda não há recordes */
    while (n < MAX_RECORDES &&
           fscanf(f, "%ld %23[^\n]", &r[n].pontos, r[n].nome) == 2)
        n++;
    fclose(f);
    return n;
}

static void recordes_gravar(const Recorde r[], int n)
{
    int i;
    FILE *f = fopen(ARQUIVO_RECORDES, "w");
    if (f == NULL) return;
    for (i = 0; i < n; i++)
        fprintf(f, "%ld %s\n", r[i].pontos, r[i].nome);
    fclose(f);
}

void recordes_registrar(long pontos)
{
    Recorde r[MAX_RECORDES + 1];
    int n = recordes_ler(r), pos = n;

    while (pos > 0 && r[pos - 1].pontos < pontos)  /* acha a posição */
        pos--;
    if (pos >= MAX_RECORDES) return;               /* não entrou */

    memmove(&r[pos + 1], &r[pos], (size_t)(n - pos) * sizeof r[0]);
    r[pos].pontos = pontos;
    if (!ui_ler_texto("Novo recorde! Seu nome:", r[pos].nome,
                      sizeof r[pos].nome))
        strcpy(r[pos].nome, "Anônimo");
    if (n < MAX_RECORDES) n++;
    recordes_gravar(r, n);
    recordes_mostrar();
}

void recordes_mostrar(void)
{
    Recorde r[MAX_RECORDES];
    char texto[512] = "";
    int n = recordes_ler(r), i;
    if (n == 0) strcpy(texto, "Ninguém saiu vivo do templo... ainda.");
    for (i = 0; i < n; i++) {
        char linha[64];
        sprintf(linha, "%d. %-20s %8ld\n", i + 1, r[i].nome, r[i].pontos);
        strcat(texto, linha);
    }
    ui_mensagem("Recordes", texto);
}
