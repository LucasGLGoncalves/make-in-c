/* tabela_comandos.c — ponteiros para função: uma tabela de comandos. */
#include <stdio.h>
#include <string.h>

typedef struct {
    int ouro;
    int vida;
} Heroi;

/* Todos os comandos têm a mesma "assinatura": recebem o herói e o objeto. */
typedef void (*Acao)(Heroi *h, const char *objeto);

void cmd_olhar(Heroi *h, const char *objeto)
{
    (void)h;
    printf("Você olha %s. Nada de especial.\n", objeto[0] ? objeto : "em volta");
}

void cmd_cavar(Heroi *h, const char *objeto)
{
    (void)objeto;
    h->ouro += 3;
    printf("Você cava e acha 3 moedas. Total: %d.\n", h->ouro);
}

void cmd_status(Heroi *h, const char *objeto)
{
    (void)objeto;
    printf("Vida %d, ouro %d.\n", h->vida, h->ouro);
}

typedef struct {
    const char *verbo;
    Acao        acao;
} Comando;

static const Comando COMANDOS[] = {
    { "olhar",  cmd_olhar  },
    { "cavar",  cmd_cavar  },
    { "status", cmd_status },
};
#define N_COMANDOS ((int)(sizeof COMANDOS / sizeof COMANDOS[0]))

int main(void)
{
    Heroi h = { 0, 10 };
    char linha[80], verbo[20], objeto[20];
    printf("Comandos: olhar, cavar, status, sair\n");
    for (;;) {
        int i, achou = 0;
        printf("> ");
        if (!fgets(linha, sizeof linha, stdin)) break;
        verbo[0] = objeto[0] = '\0';
        sscanf(linha, "%19s %19s", verbo, objeto);
        if (strcmp(verbo, "sair") == 0) break;
        for (i = 0; i < N_COMANDOS; i++)
            if (strcmp(verbo, COMANDOS[i].verbo) == 0) {
                COMANDOS[i].acao(&h, objeto);     /* chama pela tabela */
                achou = 1;
            }
        if (!achou && verbo[0]) printf("Não conheço \"%s\".\n", verbo);
    }
    return 0;
}
