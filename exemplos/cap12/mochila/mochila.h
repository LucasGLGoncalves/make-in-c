/* mochila.h — a "vitrine" do módulo: o que os outros arquivos podem usar. */
#ifndef MOCHILA_H
#define MOCHILA_H

#define MOCHILA_MAX 8

typedef struct {
    char nomes[MOCHILA_MAX][24];
    int  quantidade;
} Mochila;

void mochila_iniciar(Mochila *m);
int  mochila_por(Mochila *m, const char *nome);
int  mochila_tirar(Mochila *m, const char *nome);
void mochila_listar(const Mochila *m);
int  mochila_total_criadas(void);

#endif
