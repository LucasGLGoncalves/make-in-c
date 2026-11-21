/*
 * dados.c — leitura de arquivos de texto de dados.
 */
#include <stdlib.h>
#include <string.h>
#include "dados.h"

/* Lê uma linha inteira, sem o '\n' (e sem o '\r' do Windows).
 * Devolve 0 quando o arquivo acabou. */
int dados_ler_linha_crua(FILE *f, char *buffer, int tamanho)
{
    size_t n;
    if (fgets(buffer, tamanho, f) == NULL) return 0;
    n = strlen(buffer);
    if (n > 0 && buffer[n - 1] != '\n' && !feof(f)) {
        int c;                          /* linha longa demais: descarta */
        while ((c = fgetc(f)) != '\n' && c != EOF) { }
    }
    while (n > 0 && (buffer[n - 1] == '\n' || buffer[n - 1] == '\r'))
        buffer[--n] = '\0';
    return 1;
}

/* Como a anterior, mas pula linhas vazias e comentários (#). */
int dados_ler_linha(FILE *f, char *buffer, int tamanho)
{
    while (dados_ler_linha_crua(f, buffer, tamanho)) {
        char *p = buffer;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != '\0' && *p != '#') return 1;
    }
    return 0;
}

/* Tira espaços e tabulações do começo e do fim (altera a string). */
char *dados_aparar(char *s)
{
    char *fim;
    while (*s == ' ' || *s == '\t') s++;
    fim = s + strlen(s);
    while (fim > s && (fim[-1] == ' ' || fim[-1] == '\t')) fim--;
    *fim = '\0';
    return s;
}

/* Corta a linha nos separadores, guardando um ponteiro para cada campo.
 * Campos vazios ("a||b") são mantidos. Devolve quantos campos achou. */
int dados_separar(char *linha, char separador, char *campos[], int max)
{
    int n = 0;
    char *p = linha;
    while (n < max) {
        char *sep = strchr(p, separador);
        if (sep != NULL) *sep = '\0';
        campos[n++] = dados_aparar(p);
        if (sep == NULL) break;
        p = sep + 1;
    }
    return n;
}

char *texto_duplicar(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copia = malloc(n);
    if (copia != NULL) memcpy(copia, s, n);
    return copia;
}

int texto_comeca(const char *s, const char *prefixo)
{
    return strncmp(s, prefixo, strlen(prefixo)) == 0;
}
