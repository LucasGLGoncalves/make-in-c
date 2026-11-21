/* conversa.c — rodando um diálogo de arquivo com condições e efeitos. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "dados.h"
#include "dialogo.h"

typedef struct {
    int  ouro;
    char flags[8][24];
    int  n_flags;
} Estado;

static int tem_flag(const Estado *e, const char *nome)
{
    int i;
    for (i = 0; i < e->n_flags; i++)
        if (strcmp(e->flags[i], nome) == 0) return 1;
    return 0;
}

/* O motor pergunta: "esta condição é verdadeira?" */
static int condicao(void *ctx, const char *c)
{
    Estado *e = ctx;
    if (texto_comeca(c, "flag:")) return tem_flag(e, c + 5);
    return 0;
}

/* O motor manda: "execute este comando". */
static void efeito(void *ctx, const char *cmd)
{
    Estado *e = ctx;
    if (texto_comeca(cmd, "ouro ")) e->ouro += atoi(cmd + 5);
    if (texto_comeca(cmd, "flag ") && e->n_flags < 8)
        snprintf(e->flags[e->n_flags++], 24, "%s", cmd + 5);
}

int main(void)
{
    Dialogos d = { 0 };
    Estado e = { 20, { "" }, 0 };
    if (!dialogos_carregar(&d, "conversa.txt")) {
        printf("Não achei conversa.txt\n");
        return 1;
    }
    if (dialogos_verificar(&d, stdout) > 0) return 1;   /* destinos quebrados? */
    term_iniciar();
    tela_limpar();
    tela_texto_centro(3, AMARELO, PRETO, "O portão da cidade");
    tela_mostrar();
    dialogo_executar(&d, "guarda", "Guarda", condicao, efeito, &e);
    term_encerrar();
    printf("Fim da conversa. Ouro: %d. Flags:", e.ouro);
    for (int i = 0; i < e.n_flags; i++) printf(" %s", e.flags[i]);
    printf("\n");
    dialogos_liberar(&d);
    return 0;
}
