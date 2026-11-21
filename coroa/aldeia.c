/*
 * aldeia.c — os moradores de Vale Cinzento: conversas, lojas e flags.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tela.h"
#include "ui.h"
#include "dados.h"
#include "dialogo.h"
#include "coroa.h"

typedef struct {
    char         simbolo;    /* letra no arquivo da aldeia */
    const char  *nome;
    int          cor;
    const char  *no_inicial; /* onde a conversa começa em dialogos.txt */
} Npc;

static const Npc NPCS[] = {
    { 'a', "Ancião Tobias",      CIANO_CLARO,    "anciao"      },
    { 'f', "Bruna, a ferreira",  VERMELHO_CLARO, "ferreira"    },
    { 'c', "Mãe Ilda",           VERDE_CLARO,    "curandeira"  },
    { 't', "Gaspar, taberneiro", AMARELO,        "taberneiro"  },
    { 'g', "Guarda Rui",         AZUL_CLARO,     "guarda"      }
};
#define N_NPCS ((int)(sizeof NPCS / sizeof NPCS[0]))

static Dialogos dialogos;

int  npc_por_simbolo(char c)
{
    int i;
    for (i = 0; i < N_NPCS; i++)
        if (NPCS[i].simbolo == c) return i;
    return -1;
}
unsigned int npc_glifo(int npc)  { (void)npc; return G_ROSTO; }
int          npc_cor(int npc)    { return NPCS[npc].cor; }
const char  *npc_nome(int npc)   { return NPCS[npc].nome; }

int aldeia_iniciar(void)
{
    return dialogos_carregar(&dialogos, "dados/dialogos.txt");
}

void aldeia_encerrar(void)
{
    dialogos_liberar(&dialogos);
}

int flag_tem(const Jogo *j, const char *nome)
{
    int i;
    for (i = 0; i < j->n_flags; i++)
        if (strcmp(j->flags[i], nome) == 0) return 1;
    return 0;
}

void flag_por(Jogo *j, const char *nome)
{
    if (flag_tem(j, nome) || j->n_flags >= MAX_FLAGS) return;
    snprintf(j->flags[j->n_flags++], sizeof j->flags[0], "%s", nome);
}

static void vender(Jogo *j)
{
    Heroi *h = &j->heroi;
    const char *opcoes[MAX_MOCHILA];
    char textos[MAX_MOCHILA][128];
    int indices[MAX_MOCHILA], n = 0, i, e;

    for (i = 0; i < h->n_mochila; i++) {
        const ItemDef *d = &ITENS[h->mochila[i].def];
        if (h->mochila[i].equipado || d->tipo == IT_COROA) continue;
        char coluna[96];
        utf8_ajustar(coluna, sizeof coluna, d->nome, 22);
        snprintf(textos[n], sizeof textos[n], "%s %4d ouro", coluna, d->valor / 2);
        opcoes[n] = textos[n];
        indices[n++] = i;
    }
    if (n == 0) {
        ui_mensagem("Vender", "Você não tem nada para vender (itens "
                    "equipados não contam).");
        return;
    }
    e = ui_menu("Vender (metade do preço)", opcoes, n, 0);
    if (e < 0) return;
    i = indices[e];
    h->ouro += ITENS[h->mochila[i].def].valor / 2;
    log_adicionar(&j->log, AMARELO, "Você vende: %s.", ITENS[h->mochila[i].def].nome);
    mochila_remover(h, i, 1);
}

static void loja(Jogo *j, const char *titulo, const char *const ids[], int n)
{
    Heroi *h = &j->heroi;
    int ultima = 0;
    for (;;) {
        const char *opcoes[12];
        char textos[12][128], cabecalho[64], coluna[96];
        int defs[12], i, e;
        for (i = 0; i < n; i++) {
            defs[i] = item_por_id(ids[i]);
            utf8_ajustar(coluna, sizeof coluna, ITENS[defs[i]].nome, 22);
            snprintf(textos[i], sizeof textos[i], "%s %4d ouro",
                     coluna, ITENS[defs[i]].valor);
            opcoes[i] = textos[i];
        }
        opcoes[n] = "Vender algo";
        opcoes[n + 1] = "Sair da loja";
        snprintf(cabecalho, sizeof cabecalho, "%s - seu ouro: %d", titulo, h->ouro);
        e = ui_menu(cabecalho, opcoes, n + 2, ultima);
        if (e < 0 || e == n + 1) return;
        ultima = e;
        if (e == n) { vender(j); continue; }
        if (h->ouro < ITENS[defs[e]].valor)
            ui_mensagem("Loja", "Você não tem ouro suficiente.");
        else if (!mochila_adicionar(h, defs[e], 1))
            ui_mensagem("Loja", "Sua mochila está cheia.");
        else {
            h->ouro -= ITENS[defs[e]].valor;
            log_adicionar(&j->log, AMARELO, "Você compra: %s.", ITENS[defs[e]].nome);
        }
    }
}

/* Responde às condições escritas entre [colchetes] em dialogos.txt. */
static int condicao(void *ctx, const char *c)
{
    Jogo *j = ctx;
    if (strcmp(c, "coroa") == 0)   return mochila_procurar(&j->heroi, "coroa") >= 0;
    if (strcmp(c, "ferido") == 0)  return j->heroi.hp < j->heroi.hp_max;
    if (texto_comeca(c, "ouro:"))  return j->heroi.ouro >= atoi(c + 5);
    if (texto_comeca(c, "flag:"))  return flag_tem(j, c + 5);
    if (texto_comeca(c, "fundo:")) return j->heroi.prof_max >= atoi(c + 6);
    return 0;                                   /* condição desconhecida */
}

/* Executa os comandos escritos depois de '!' em dialogos.txt. */
static void efeito(void *ctx, const char *cmd)
{
    static const char *const FERREIRA[] = { "adaga", "espada_curta", "machado",
        "gibao", "cota", "elmo", "escudo" };
    static const char *const CURANDEIRA[] = { "pocao_cura", "pocao_maior", "mapa" };
    Jogo *j = ctx;
    Heroi *h = &j->heroi;

    if (strcmp(cmd, "loja ferreira") == 0)
        loja(j, "Forja da Bruna", FERREIRA, 7);
    else if (strcmp(cmd, "loja curandeira") == 0)
        loja(j, "Ervas da Mãe Ilda", CURANDEIRA, 3);
    else if (strcmp(cmd, "curar") == 0)
        h->hp = h->hp_max;
    else if (texto_comeca(cmd, "ouro "))
        h->ouro += atoi(cmd + 5);               /* "ouro -10" tira 10 */
    else if (texto_comeca(cmd, "vida ")) {
        h->hp += atoi(cmd + 5);
        if (h->hp > h->hp_max) h->hp = h->hp_max;
    } else if (texto_comeca(cmd, "flag "))
        flag_por(j, cmd + 5);
    else if (texto_comeca(cmd, "item "))
        mochila_adicionar(h, item_por_id(cmd + 5), 1);
    else if (strcmp(cmd, "vitoria") == 0)
        j->fim = FIM_VITORIA;
}

void aldeia_conversar(Jogo *j, int indice_ator)
{
    int npc = j->atores[indice_ator].npc;
    dialogo_executar(&dialogos, NPCS[npc].no_inicial, NPCS[npc].nome,
                     condicao, efeito, j);
}
