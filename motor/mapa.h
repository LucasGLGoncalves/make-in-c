/*
 * mapa.h — mapas feitos de tiles, carregados de arquivos de texto,
 * e a câmera que escolhe qual pedaço do mapa aparece na tela.
 */
#ifndef MAPA_H
#define MAPA_H

#include <stdio.h>

/* Propriedades de um tile, combinadas como bits (capítulo 14).
 * Os jogos podem criar as suas a partir de (1u << 8). */
enum {
    T_SOLIDO = 1u << 0,   /* ninguém atravessa          */
    T_OPACO  = 1u << 1    /* a visão não atravessa      */
};

typedef struct {
    unsigned int  glifo;
    unsigned char frente, fundo;
    unsigned int  flags;
    const char   *nome;
} TileDef;

typedef struct {
    int            largura, altura;
    unsigned char *tiles;     /* índice de cada célula na tabela de tiles */
    unsigned char *visivel;   /* 1 = o jogador vê esta célula agora        */
    unsigned char *visto;     /* 1 = o jogador já viu esta célula um dia   */
    const TileDef *defs;      /* a tabela de tiles do jogo                 */
} Mapa;

typedef struct {
    int x, y;                    /* canto superior esquerdo, no mapa */
    int tela_x, tela_y;          /* onde a janela fica na tela       */
    int largura, altura;         /* tamanho da janela                */
} Camera;

/* Chamada para cada símbolo que não está na legenda (ex.: '@').
 * Deve devolver o índice do tile que fica embaixo do símbolo. */
typedef int (*MapaSimbolo)(void *ctx, char simbolo, int x, int y);

int          mapa_criar(Mapa *m, int largura, int altura, const TileDef *defs);
void         mapa_destruir(Mapa *m);
int          mapa_dentro(const Mapa *m, int x, int y);
int          mapa_tile(const Mapa *m, int x, int y);   /* -1 se fora */
void         mapa_por(Mapa *m, int x, int y, int tile);
void         mapa_preencher(Mapa *m, int tile);
unsigned int mapa_flags(const Mapa *m, int x, int y);
int          mapa_bloqueia(const Mapa *m, int x, int y);
int          mapa_opaco(const Mapa *m, int x, int y);

int  mapa_ler(Mapa *m, FILE *f, const TileDef *defs, const char *legenda,
              MapaSimbolo desconhecido, void *ctx);
int  mapa_carregar(Mapa *m, const char *arquivo, const TileDef *defs,
                   const char *legenda, MapaSimbolo desconhecido, void *ctx);

void camera_centralizar(Camera *c, const Mapa *m, int alvo_x, int alvo_y);
int  camera_na_tela(const Camera *c, int mx, int my, int *tx, int *ty);
void mapa_desenhar(const Mapa *m, const Camera *c, int usar_fov);

#endif
