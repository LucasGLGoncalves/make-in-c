/*
 * dados.h — ferramentas para ler arquivos de texto de dados:
 * linhas, campos separados por '|', espaços nas pontas, comentários.
 */
#ifndef DADOS_H
#define DADOS_H

#include <stdio.h>

int   dados_ler_linha_crua(FILE *f, char *buffer, int tamanho);
int   dados_ler_linha(FILE *f, char *buffer, int tamanho);
char *dados_aparar(char *s);
int   dados_separar(char *linha, char separador, char *campos[], int max);
char *texto_duplicar(const char *s);
int   texto_comeca(const char *s, const char *prefixo);

#endif
