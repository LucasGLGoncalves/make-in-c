/* comandos.c — lendo frases e separando verbo e objeto. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Lê uma linha do teclado, sem o '\n' do final. */
void ler_linha(char *destino, int tamanho)
{
    size_t n;
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }
    n = strlen(destino);
    if (n > 0 && destino[n - 1] == '\n') destino[n - 1] = '\0';
}

int main(void)
{
    char linha[100], verbo[30], objeto[30];
    int i;

    printf("Escreva um comando (ex.: pegar chave). 'sair' termina.\n");
    for (;;) {
        printf("> ");
        ler_linha(linha, sizeof linha);
        for (i = 0; linha[i] != '\0'; i++)               /* tudo minúsculo */
            linha[i] = (char)tolower((unsigned char)linha[i]);

        verbo[0] = objeto[0] = '\0';
        sscanf(linha, "%29s %29s", verbo, objeto);      /* 1ª e 2ª palavras */

        if (strcmp(verbo, "sair") == 0) break;
        if (verbo[0] == '\0') continue;
        printf("Verbo: \"%s\"  Objeto: \"%s\"  (%d letras no verbo)\n",
               verbo, objeto, (int)strlen(verbo));
        if (strcmp(verbo, "pegar") == 0 && strcmp(objeto, "chave") == 0)
            printf("Você pega a chave enferrujada.\n");
    }
    return 0;
}
