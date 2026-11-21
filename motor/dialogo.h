/*
 * dialogo.h — árvores de diálogo lidas de arquivos de texto.
 *
 * Formato do arquivo (veja o capítulo 22):
 *
 *   # comentário
 *   :identificador          começa um nó
 *   @Nome de quem fala      (opcional) título da janela
 *   ! comando               (opcional) efeito ao entrar no nó
 *   Qualquer outra linha    texto do nó
 *   * [condição] Texto -> destino     uma opção ("FIM" encerra)
 */
#ifndef DIALOGO_H
#define DIALOGO_H

#include <stdio.h>

#define DLG_MAX_OPCOES 8

typedef struct {
    char *texto;
    char *condicao;    /* "" = sempre visível */
    char *destino;
} DlgOpcao;

typedef struct {
    char    *id;
    char    *falante;  /* "" = usa o título padrão */
    char    *texto;
    char    *efeitos;  /* comandos separados por '\n' */
    DlgOpcao opcoes[DLG_MAX_OPCOES];
    int      n_opcoes;
} DlgNo;

typedef struct {
    DlgNo *nos;
    int    n, capacidade;
} Dialogos;

/* O jogo responde se uma condição simples é verdadeira... */
typedef int  (*DlgCondicao)(void *ctx, const char *condicao);
/* ...e executa os comandos dos nós ("dar ouro 10", "pista faca"...). */
typedef void (*DlgEfeito)(void *ctx, const char *comando);

int          dialogos_carregar(Dialogos *d, const char *arquivo);
void         dialogos_liberar(Dialogos *d);
const DlgNo *dialogos_buscar(const Dialogos *d, const char *id);
int          dialogos_verificar(const Dialogos *d, FILE *relatorio);
int          dialogo_condicao(const char *condicao, DlgCondicao f, void *ctx);
void         dialogo_executar(const Dialogos *d, const char *inicio,
                              const char *titulo, DlgCondicao condicao,
                              DlgEfeito efeito, void *ctx);

#endif
