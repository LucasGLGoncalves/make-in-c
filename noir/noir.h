/*
 * noir.h — "Porto Sombrio": tipos e funções compartilhadas.
 */
#ifndef NOIR_H
#define NOIR_H

#include "mapa.h"
#include "ui.h"

#define VISTA_A        18     /* linhas do mapa na tela              */
#define LOG_Y          19     /* onde começa o registro de mensagens */
#define MAX_COISAS     16     /* pessoas e objetos por local         */
#define MAX_PISTAS     16
#define MAX_FLAGS      48
#define MAX_GOTAS      80     /* gotas de chuva animadas             */
#define PRAZO_MINUTOS  600    /* das 20h00 às 6h00                   */

enum {
    NT_VAZIO, NT_PAREDE, NT_PISO, NT_RUA, NT_CARPETE, NT_PORTA, NT_BALCAO,
    NT_JANELA, NT_MESA, NT_NEON, NT_MAR, NT_SAIDA, NT_PALCO, NT_CAMA,
    NT_ESTANTE, NT_PIANO
};
#define T_EXTERNO (1u << 8)   /* recebe chuva  */
#define T_NEON    (1u << 9)   /* pisca         */

typedef struct {               /* uma pessoa ou objeto do local */
    char         letra;
    char         nome[40];
    unsigned int glifo;
    int          cor;
    char         no[32];       /* nó de diálogo                 */
    char         condicao[64]; /* só aparece se for verdadeira  */
    int          x, y;
} Coisa;

typedef struct {
    char id[20];
    char titulo[48];
    char texto[256];
} Pista;

typedef struct { int x, y; } Gota;

typedef enum { INVESTIGANDO, FINAL_BOM, FINAL_FRACO, FINAL_ERRADO,
               FINAL_PRAZO, DESISTIU } Situacao;

typedef struct {
    Mapa     mapa;
    Camera   cam;
    char     local[24];          /* nome do arquivo do local atual */
    char     nome_local[48];
    Coisa    coisas[MAX_COISAS];
    int      n_coisas;
    int      x, y;               /* a detetive */
    int      minutos;            /* desde as 20h00 de sexta */
    int      dinheiro;
    char     pistas[MAX_PISTAS][20];
    int      n_pistas;
    char     flags[MAX_FLAGS][24];
    int      n_flags;
    Gota     gotas[MAX_GOTAS];
    Log      log;
    Situacao situacao;
} Caso;

/* locais.c */
extern const TileDef TILES[];
int  local_carregar(Caso *c, const char *nome, int de_taxi);
int  coisa_presente(Caso *c, int i);
int  coisa_em(Caso *c, int x, int y);
void chuva_iniciar(Caso *c);
void chuva_atualizar(Caso *c);

/* caso.c */
int  caso_iniciar(void);
void caso_encerrar(void);
int  caso_condicao(void *ctx, const char *cond);
void caso_interagir(Caso *c, int indice_coisa);
void caso_taxi(Caso *c);
void caso_caderno(const Caso *c);
int  caso_total_pistas(void);
void caso_hora(const Caso *c, char *texto);

/* desenho.c */
void desenhar_caso(Caso *c);

#endif
