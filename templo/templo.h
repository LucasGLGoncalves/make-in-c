/*
 * templo.h — tipos e funções compartilhados por "O Templo da Serpente".
 */
#ifndef TEMPLO_H
#define TEMPLO_H

#include "mapa.h"

#define CAMPO_L       64    /* largura do campo de jogo (e dos níveis)  */
#define CAMPO_A       23    /* altura do campo de jogo                  */
#define PAINEL_X      64    /* onde começa o painel lateral             */
#define TICKS_SEG     20    /* atualizações da lógica por segundo       */
#define MAX_MONSTROS  200
#define TOTAL_NIVEIS  4

/* A ordem aqui é a mesma da LEGENDA em niveis.c. */
enum {
    TILE_CHAO, TILE_PAREDE, TILE_QUEBRAVEL, TILE_MATO, TILE_PORTA,
    TILE_AGUA, TILE_LAVA, TILE_GEMA, TILE_CHICOTE, TILE_CHAVE,
    TILE_ORBE, TILE_TESOURO, TILE_SAIDA, TILE_IDOLO
};

#define T_LAVA (1u << 8)   /* flag própria do jogo: queima quem pisa */

typedef struct {
    int x, y;
    int tipo;       /* 0, 1 ou 2: rastejante, guardião, serpente */
    int vivo;
    int espera;     /* ticks até o próximo passo */
} Monstro;

typedef struct {
    int x, y;
    int gemas, chicotes, chaves, orbes;
    long pontos;
} Jogador;

typedef enum { RODANDO, MORREU, VENCEU, DESISTIU } Situacao;

typedef struct {
    Mapa     mapa;
    Monstro  monstros[MAX_MONSTROS];
    int      n_monstros;
    Jogador  jog;
    int      nivel;              /* 1 a TOTAL_NIVEIS */
    char     titulo[80];
    char     aviso[80];          /* mensagem na linha de baixo */
    int      aviso_ticks;        /* por quantos ticks ela fica  */
    long     tick;
    Situacao situacao;
} Jogo;

/* niveis.c */
extern const TileDef TILES[];
int  nivel_carregar(Jogo *j, int numero);

/* jogador.c */
void jogador_iniciar(Jogador *p);
void jogador_mover(Jogo *j, int dx, int dy);
void jogador_chicotear(Jogo *j);
void jogador_teleportar(Jogo *j);
void avisar(Jogo *j, const char *texto);

/* monstros.c */
void monstros_atualizar(Jogo *j);
int  monstro_em(const Jogo *j, int x, int y);
void monstro_atingir_jogador(Jogo *j, Monstro *m);

/* desenho.c */
void desenhar_jogo(const Jogo *j);

/* recordes.c */
void recordes_registrar(long pontos);
void recordes_mostrar(void);

#endif
