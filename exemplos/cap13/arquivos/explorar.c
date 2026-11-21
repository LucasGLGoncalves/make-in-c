/* explorar.c — lê o mapa de um arquivo e salva o progresso em outro. */
#include <stdio.h>
#include <string.h>

#define MAX_L 80
#define MAX_A 30

char mapa[MAX_A][MAX_L + 2];
int  altura = 0;

/* Lê mundo.txt linha por linha. Devolve 1 se deu certo. */
int carregar_mapa(const char *arquivo, int *x, int *y)
{
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) {
        perror(arquivo);                     /* explica o erro do sistema */
        return 0;
    }
    while (altura < MAX_A && fgets(mapa[altura], sizeof mapa[0], f) != NULL) {
        char *arroba;
        mapa[altura][strcspn(mapa[altura], "\r\n")] = '\0';   /* tira o \n */
        arroba = strchr(mapa[altura], '@');
        if (arroba != NULL) {
            *x = (int)(arroba - mapa[altura]);
            *y = altura;
            *arroba = '.';
        }
        altura++;
    }
    fclose(f);
    return altura > 0;
}

void salvar(int x, int y, int ouro)
{
    FILE *f = fopen("progresso.txt", "w");
    if (f == NULL) return;
    fprintf(f, "%d %d %d\n", x, y, ouro);
    fclose(f);
}

int carregar_progresso(int *x, int *y, int *ouro)
{
    FILE *f = fopen("progresso.txt", "r");
    int ok;
    if (f == NULL) return 0;
    ok = fscanf(f, "%d %d %d", x, y, ouro) == 3;
    fclose(f);
    return ok;
}

int main(void)
{
    int x = 1, y = 1, ouro = 0, j;
    char tecla;
    if (!carregar_mapa("mundo.txt", &x, &y)) return 1;
    if (carregar_progresso(&x, &y, &ouro))
        printf("Progresso carregado! Você tem %d de ouro.\n", ouro);

    for (;;) {
        int nx = x, ny = y;
        for (j = 0; j < altura; j++) {
            int i;
            for (i = 0; mapa[j][i] != '\0'; i++)
                putchar(i == x && j == y ? '@' : mapa[j][i]);
            putchar('\n');
        }
        printf("Ouro %d. w/a/s/d anda, g grava, q sai: ", ouro);
        if (scanf(" %c", &tecla) != 1 || tecla == 'q') break;
        if (tecla == 'g') { salvar(x, y, ouro); printf("Gravado!\n"); continue; }
        if (tecla == 'w') ny--;
        if (tecla == 's') ny++;
        if (tecla == 'a') nx--;
        if (tecla == 'd') nx++;
        if (strchr("#~", mapa[ny][nx]) != NULL) continue;  /* parede ou água */
        x = nx;
        y = ny;
        if (mapa[y][x] == '$') { ouro += 10; mapa[y][x] = '.'; }
    }
    return 0;
}
