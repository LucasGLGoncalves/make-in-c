/*
 * caso.c — pistas, flags, relógio, diálogos, táxi e caderno.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "dados.h"
#include "dialogo.h"
#include "noir.h"

static Dialogos dialogos;
static Pista    PISTAS[MAX_PISTAS];
static int      N_PISTAS = 0;

static int carregar_pistas(const char *arquivo)
{
    char linha[512], *campo[3];
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;
    while (N_PISTAS < MAX_PISTAS && dados_ler_linha(f, linha, sizeof linha)) {
        Pista *p = &PISTAS[N_PISTAS];
        if (dados_separar(linha, '|', campo, 3) != 3) continue;
        snprintf(p->id, sizeof p->id, "%s", campo[0]);
        snprintf(p->titulo, sizeof p->titulo, "%s", campo[1]);
        snprintf(p->texto, sizeof p->texto, "%s", campo[2]);
        N_PISTAS++;
    }
    fclose(f);
    return N_PISTAS > 0;
}

int caso_iniciar(void)
{
    if (!carregar_pistas("dados/pistas.txt")) return 0;
    if (!dialogos_carregar(&dialogos, "dados/dialogos.txt")) return 0;
    return 1;
}

void caso_encerrar(void)
{
    dialogos_liberar(&dialogos);
}

int caso_total_pistas(void)
{
    return N_PISTAS;
}

/* Pistas e flags são listas de nomes: "tem" procura, "por" acrescenta. */
static int lista_tem(char lista[][24], int n, const char *nome)
{
    int i;
    for (i = 0; i < n; i++)
        if (strcmp(lista[i], nome) == 0) return 1;
    return 0;
}

static int tem_pista(const Caso *c, const char *id)
{
    int i;
    for (i = 0; i < c->n_pistas; i++)
        if (strcmp(c->pistas[i], id) == 0) return 1;
    return 0;
}

static const Pista *pista_def(const char *id)
{
    int i;
    for (i = 0; i < N_PISTAS; i++)
        if (strcmp(PISTAS[i].id, id) == 0) return &PISTAS[i];
    return NULL;
}

/* Escreve "Sex 23:40" ou "Sáb 01:15" em 'texto'. */
void caso_hora(const Caso *c, char *texto)
{
    int total = 20 * 60 + c->minutos;          /* minutos desde 0h de sexta */
    int dia = total / (24 * 60);
    int hora = (total / 60) % 24, minuto = total % 60;
    sprintf(texto, "%s %02d:%02d", dia == 0 ? "Sex" : "Sáb", hora, minuto);
}

int caso_condicao(void *ctx, const char *cond)
{
    Caso *c = ctx;
    if (texto_comeca(cond, "pista:"))    return tem_pista(c, cond + 6);
    if (texto_comeca(cond, "flag:"))     return lista_tem(c->flags, c->n_flags, cond + 5);
    if (texto_comeca(cond, "dinheiro:")) return c->dinheiro >= atoi(cond + 9);
    if (texto_comeca(cond, "pistas:"))   return c->n_pistas >= atoi(cond + 7);
    return 0;
}

static void caso_efeito(void *ctx, const char *cmd)
{
    Caso *c = ctx;
    if (texto_comeca(cmd, "pista ")) {
        const char *id = cmd + 6;
        const Pista *p = pista_def(id);
        if (p != NULL && !tem_pista(c, id) && c->n_pistas < MAX_PISTAS) {
            snprintf(c->pistas[c->n_pistas++], 20, "%s", id);
            log_adicionar(&c->log, AMARELO, "Nova pista no caderno: %s.", p->titulo);
        }
    } else if (texto_comeca(cmd, "flag ")) {
        if (!lista_tem(c->flags, c->n_flags, cmd + 5) && c->n_flags < MAX_FLAGS)
            snprintf(c->flags[c->n_flags++], 24, "%s", cmd + 5);
    } else if (texto_comeca(cmd, "tempo ")) {
        c->minutos += atoi(cmd + 6);
    } else if (texto_comeca(cmd, "dinheiro ")) {
        int valor = atoi(cmd + 9);
        c->dinheiro += valor;
        log_adicionar(&c->log, VERDE_CLARO, "%s %d pratas.",
                      valor >= 0 ? "Você recebe" : "Você paga", abs(valor));
    } else if (strcmp(cmd, "final bom") == 0)    c->situacao = FINAL_BOM;
    else if (strcmp(cmd, "final fraco") == 0)    c->situacao = FINAL_FRACO;
    else if (strcmp(cmd, "final errado") == 0)   c->situacao = FINAL_ERRADO;
}

void caso_interagir(Caso *c, int indice)
{
    Coisa *k = &c->coisas[indice];
    c->minutos += 5;                         /* toda conversa toma tempo */
    dialogo_executar(&dialogos, k->no, k->nome, caso_condicao, caso_efeito, c);
}

typedef struct {
    const char *local;      /* arquivo em dados/locais/          */
    const char *nome;
    const char *condicao;   /* quando a detetive conhece o lugar */
    int         minutos;    /* duração da corrida                */
} Destino;

static const Destino DESTINOS[] = {
    { "escritorio",  "Escritório, Rua do Carmo",      "",                       20 },
    { "boate",       "Boate Lua Azul",                "",                       20 },
    { "apartamento", "Apartamento de Celeste",        "flag:local_apartamento", 25 },
    { "consultorio", "Consultório do Dr. Sales",      "flag:local_consultorio", 30 },
    { "cais",        "Cais do Porto",                 "flag:local_cais",        35 },
    { "delegacia",   "Delegacia do 4º Distrito",      "",                       20 }
};
#define N_DESTINOS ((int)(sizeof DESTINOS / sizeof DESTINOS[0]))

void caso_taxi(Caso *c)
{
    const char *opcoes[N_DESTINOS + 1];
    char textos[N_DESTINOS][96];
    int indices[N_DESTINOS], n = 0, i, e;

    for (i = 0; i < N_DESTINOS; i++) {
        if (strcmp(DESTINOS[i].local, c->local) == 0) continue;     /* já está aqui */
        if (!dialogo_condicao(DESTINOS[i].condicao, caso_condicao, c)) continue;
        snprintf(textos[n], sizeof textos[n], "%s (%d min)",
                 DESTINOS[i].nome, DESTINOS[i].minutos);
        opcoes[n] = textos[n];
        indices[n++] = i;
    }
    opcoes[n] = "Ficar por aqui";
    e = ui_menu("Táxi! Para onde?", opcoes, n + 1, 0);
    if (e < 0 || e == n) return;

    i = indices[e];
    c->minutos += DESTINOS[i].minutos;
    local_carregar(c, DESTINOS[i].local, 1);
    log_adicionar(&c->log, CINZA, "O táxi deixa você: %s.", c->nome_local);
}

void caso_caderno(const Caso *c)
{
    int i, y = 3;
    char hora[16];
    tela_guardar();
    ui_janela(4, 0, 72, TELA_A, "Caderno de anotações", AMARELO);
    caso_hora(c, hora);
    tela_textof(7, 1, CINZA_ESCURO, PRETO,
                "%s  -  %d de %d pistas  -  prazo: Sáb 06:00",
                hora, c->n_pistas, N_PISTAS);
    if (c->n_pistas == 0)
        tela_texto(7, 3, CINZA, PRETO, "(Nenhuma pista ainda. Converse, examine, insista.)");
    for (i = 0; i < c->n_pistas && y < TELA_A - 2; i++) {
        const Pista *p = pista_def(c->pistas[i]);
        if (p == NULL) continue;
        tela_texto(7, y, AMARELO, PRETO, p->titulo);
        y += 1 + ui_texto_quebrado(9, y + 1, 64, 3, CINZA, PRETO, p->texto);
    }
    tela_mostrar();
    term_esperar_tecla();
    tela_recuperar();
}
