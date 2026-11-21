/*
 * dialogo.c — leitura e execução das árvores de diálogo.
 */
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "dados.h"
#include "dialogo.h"

/* Acrescenta 'extra' ao final de *destino (que pode ser NULL),
 * colocando 'separador' entre os dois quando já havia texto. */
static void anexar(char **destino, const char *separador, const char *extra)
{
    size_t antes = *destino ? strlen(*destino) : 0;
    size_t sep   = antes ? strlen(separador) : 0;
    char *novo   = realloc(*destino, antes + sep + strlen(extra) + 1);
    if (novo == NULL) return;
    if (antes == 0) novo[0] = '\0';
    if (sep) strcat(novo, separador);
    strcat(novo, extra);
    *destino = novo;
}

static DlgNo *novo_no(Dialogos *d, const char *id)
{
    DlgNo *no;
    if (d->n == d->capacidade) {
        int nova = d->capacidade ? d->capacidade * 2 : 32;
        DlgNo *maior = realloc(d->nos, (size_t)nova * sizeof *maior);
        if (maior == NULL) return NULL;
        d->nos = maior;
        d->capacidade = nova;
    }
    no = &d->nos[d->n++];
    memset(no, 0, sizeof *no);
    no->id      = texto_duplicar(id);
    no->falante = texto_duplicar("");
    return no;
}

/* "* [cond] Texto da opção -> destino" */
static void ler_opcao(DlgNo *no, char *linha)
{
    DlgOpcao *op;
    char *condicao = "", *seta;
    if (no->n_opcoes == DLG_MAX_OPCOES) return;
    linha = dados_aparar(linha + 1);           /* pula o '*' */
    if (linha[0] == '[') {
        char *fecha = strchr(linha, ']');
        if (fecha != NULL) {
            *fecha = '\0';
            condicao = dados_aparar(linha + 1);
            linha = dados_aparar(fecha + 1);
        }
    }
    seta = strstr(linha, "->");
    if (seta == NULL) return;                  /* opção sem destino: ignora */
    *seta = '\0';
    op = &no->opcoes[no->n_opcoes++];
    op->texto    = texto_duplicar(dados_aparar(linha));
    op->condicao = texto_duplicar(condicao);
    op->destino  = texto_duplicar(dados_aparar(seta + 2));
}

int dialogos_carregar(Dialogos *d, const char *arquivo)
{
    char   buffer[1024];
    DlgNo *no = NULL;
    int    paragrafo = 0;
    FILE  *f = fopen(arquivo, "r");

    if (d->nos == NULL) { d->n = 0; d->capacidade = 0; }
    if (f == NULL) return 0;

    while (dados_ler_linha_crua(f, buffer, sizeof buffer)) {
        char *linha = dados_aparar(buffer);
        if (linha[0] == '#') continue;
        if (linha[0] == '\0') { paragrafo = 1; continue; }
        if (linha[0] == ':') {
            no = novo_no(d, dados_aparar(linha + 1));
            paragrafo = 0;
            continue;
        }
        if (no == NULL) continue;              /* texto antes do 1º nó */
        if (linha[0] == '@') {
            free(no->falante);
            no->falante = texto_duplicar(dados_aparar(linha + 1));
        } else if (linha[0] == '!') {
            anexar(&no->efeitos, "\n", dados_aparar(linha + 1));
        } else if (linha[0] == '*') {
            ler_opcao(no, linha);
        } else {
            anexar(&no->texto, paragrafo ? "\n\n" : "\n", linha);
            paragrafo = 0;
        }
    }
    fclose(f);
    return 1;
}

void dialogos_liberar(Dialogos *d)
{
    int i, j;
    for (i = 0; i < d->n; i++) {
        DlgNo *no = &d->nos[i];
        free(no->id); free(no->falante); free(no->texto); free(no->efeitos);
        for (j = 0; j < no->n_opcoes; j++) {
            free(no->opcoes[j].texto);
            free(no->opcoes[j].condicao);
            free(no->opcoes[j].destino);
        }
    }
    free(d->nos);
    d->nos = NULL;
    d->n = d->capacidade = 0;
}

const DlgNo *dialogos_buscar(const Dialogos *d, const char *id)
{
    int i;
    for (i = 0; i < d->n; i++)
        if (strcmp(d->nos[i].id, id) == 0) return &d->nos[i];
    return NULL;
}

/* Procura destinos que não existem. Devolve o número de problemas. */
int dialogos_verificar(const Dialogos *d, FILE *relatorio)
{
    int i, j, problemas = 0;
    for (i = 0; i < d->n; i++)
        for (j = 0; j < d->nos[i].n_opcoes; j++) {
            const char *destino = d->nos[i].opcoes[j].destino;
            if (strcmp(destino, "FIM") != 0 && !dialogos_buscar(d, destino)) {
                fprintf(relatorio, "nó '%s': destino '%s' não existe\n",
                        d->nos[i].id, destino);
                problemas++;
            }
        }
    return problemas;
}

/* "tem_foto !falou_com_tiao" = tem_foto E NÃO falou_com_tiao */
int dialogo_condicao(const char *condicao, DlgCondicao f, void *ctx)
{
    char atomo[64];
    const char *p = condicao;
    while (*p != '\0') {
        int n = 0, negar = 0, resultado;
        while (*p == ' ') p++;
        if (*p == '\0') break;
        if (*p == '!') { negar = 1; p++; }
        while (*p != '\0' && *p != ' ' && n < (int)sizeof atomo - 1)
            atomo[n++] = *p++;
        atomo[n] = '\0';
        resultado = (f != NULL) ? f(ctx, atomo) : 0;
        if (negar) resultado = !resultado;
        if (!resultado) return 0;
    }
    return 1;
}

static void rodar_efeitos(const DlgNo *no, DlgEfeito efeito, void *ctx)
{
    char comando[256];
    const char *p = no->efeitos;
    if (p == NULL || efeito == NULL) return;
    while (*p != '\0') {
        int n = 0;
        while (*p != '\0' && *p != '\n' && n < (int)sizeof comando - 1)
            comando[n++] = *p++;
        comando[n] = '\0';
        if (*p == '\n') p++;
        efeito(ctx, comando);
    }
}

void dialogo_executar(const Dialogos *d, const char *inicio,
                      const char *titulo, DlgCondicao condicao,
                      DlgEfeito efeito, void *ctx)
{
    const DlgNo *no = dialogos_buscar(d, inicio);
    const char *nome = titulo;          /* quem fala, até um '@' mudar */
    tela_guardar();

    while (no != NULL) {
        const DlgOpcao *visiveis[DLG_MAX_OPCOES];
        char linhas[16][UI_LINHA];
        int n_vis = 0, n_linhas, escolha = 0, i, tecla;
        int altura, y, l = 76, x = 2;
        if (no->falante[0] != '\0') nome = no->falante;

        tela_recuperar();            /* efeitos (como uma loja) abrem por cima */
        tela_guardar();              /* do jogo, e não da janela anterior      */
        rodar_efeitos(no, efeito, ctx);
        for (i = 0; i < no->n_opcoes; i++)
            if (dialogo_condicao(no->opcoes[i].condicao, condicao, ctx))
                visiveis[n_vis++] = &no->opcoes[i];
        n_linhas = ui_quebrar(no->texto ? no->texto : "", l - 4, linhas, 16);
        altura = n_linhas + (n_vis > 0 ? n_vis + 1 : 0) + 4;
        y = TELA_A - altura;
        if (y < 0) y = 0;

        for (;;) {                               /* desenha e espera */
            tela_recuperar();
            tela_guardar();
            ui_janela(x, y, l, altura, nome, CIANO);
            for (i = 0; i < n_linhas; i++)
                tela_texto(x + 2, y + 2 + i, BRANCO, PRETO, linhas[i]);
            for (i = 0; i < n_vis; i++) {
                int linha = y + 3 + n_linhas + i;
                int marcada = (i == escolha);
                tela_retangulo(x + 1, linha, l - 2, 1, ' ', CINZA,
                               marcada ? AZUL : PRETO);
                tela_textof(x + 2, linha, AMARELO, marcada ? AZUL : PRETO,
                            "%d)", i + 1);
                tela_texto(x + 5, linha, marcada ? BRANCO : CINZA,
                           marcada ? AZUL : PRETO, visiveis[i]->texto);
            }
            if (n_vis == 0)
                tela_texto(x + l - 10, y + altura - 1, CINZA_ESCURO, PRETO,
                           "[tecla]");
            tela_mostrar();

            tecla = term_esperar_tecla();
            if (n_vis == 0) break;
            if (tecla == TECLA_CIMA)  escolha = (escolha + n_vis - 1) % n_vis;
            if (tecla == TECLA_BAIXO) escolha = (escolha + 1) % n_vis;
            if (tecla >= '1' && tecla < '1' + n_vis) { escolha = tecla - '1'; break; }
            if (tecla == TECLA_ENTER || tecla == ' ') break;
        }

        if (n_vis == 0 || strcmp(visiveis[escolha]->destino, "FIM") == 0)
            no = NULL;
        else
            no = dialogos_buscar(d, visiveis[escolha]->destino);
    }
    tela_recuperar();
    tela_mostrar();
}
