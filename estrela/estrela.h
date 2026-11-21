/*
 * estrela.h — "Estrela Errante": tipos e funções compartilhadas.
 */
#ifndef ESTRELA_H
#define ESTRELA_H

#include "mapa.h"
#include "ui.h"

#define N_BENS          8
#define MAX_SISTEMAS    10
#define MAX_PIRATAS     8
#define MAX_TIROS       64
#define MAX_PARTICULAS  120
#define MAX_CONTAINERS  16
#define MAX_MISSOES     3
#define MAX_FLAGS       24
#define DIVIDA_INICIAL  20000
#define PRAZO_DIAS      100
#define TANQUE          10      /* unidades de combustível */
#define TICKS_SEG       30

typedef enum { ECO_AGRICOLA, ECO_MINERADORA, ECO_INDUSTRIAL, ECO_TECNOLOGICA,
               ECO_RICA, ECO_FORA_DA_LEI, N_ECONOMIAS } Economia;

typedef struct {
    char id[20];
    char nome[32];
    int  x, y;                  /* posição no mapa estelar */
    int  economia;
    int  perigo;                /* 0 = seguro ... 3 = covil de piratas */
    char descricao[200];
    int  preco[N_BENS];         /* preços de hoje, sorteados na chegada */
} Sistema;

typedef enum { EQ_ARMA, EQ_ESCUDO, EQ_MOTOR, EQ_PORAO, EQ_CASCO, N_EQUIP } Equip;

typedef struct {
    int destino, bem, qtd, pagamento, prazo;
} Missao;

/* Tudo o que é salvo no arquivo: a "ficha" da capitã e da nave. */
typedef struct {
    char   nome[24];
    int    sistema;
    int    dia;
    long   creditos, divida;
    int    equip[N_EQUIP];      /* nível de cada peça: 0, 1 ou 2 */
    int    casco;               /* integridade atual            */
    int    combustivel;
    int    carga[N_BENS];
    Missao missoes[MAX_MISSOES];
    int    n_missoes;
    char   flags[MAX_FLAGS][24];
    int    n_flags;
    int    abates;
} Piloto;

typedef struct { double x, y, vx, vy; int dir; double escudo; int recarga, acelerador, ultimo_giro; } Nave;
typedef struct { double x, y, vx, vy; int dir, hp, recarga, ativo, giro; } Pirata;
typedef struct { double x, y, vx, vy; int ttl, do_jogador, ativo, dir; } Tiro;
typedef struct { double x, y, vx, vy; int ttl; } Particula;
typedef struct { double x, y; int bem, qtd, ativo; } Container;

typedef enum { MODO_ESPACO, MODO_ESTACAO, MODO_FIM } Modo;
typedef enum { EM_JOGO, FIM_VITORIA, FIM_DESTRUIDA, FIM_PRAZO, FIM_SAIU } Fim;

typedef struct {
    Piloto    p;
    Nave      nave;
    Pirata    piratas[MAX_PIRATAS];
    Tiro      tiros[MAX_TIROS];
    Particula part[MAX_PARTICULAS];
    Container cont[MAX_CONTAINERS];
    Log       log;
    long      tick;
    Mapa      estacao;          /* o interior da estação (a pé) */
    Camera    cam;
    int       ex, ey;           /* a capitã dentro da estação   */
    Missao    oferta;           /* missão oferecida no bar      */
    int       tem_oferta;
    Fim       fim;
} Jogo;

/* universo.c */
extern Sistema SISTEMAS[MAX_SISTEMAS];
extern int     N_SISTEMAS;
extern const char *const NOME_BEM[N_BENS];
int  universo_carregar(void);
void universo_sortear_precos(int sistema);
int  preco_venda(int sistema, int bem);
int  distancia_salto(int a, int b);
int  mapa_estelar(Jogo *j);          /* devolve o sistema escolhido ou -1 */
void saltar(Jogo *j, int destino);

/* nave.c */
const char *equip_nome(int tipo, int nivel);
int  equip_valor(int tipo, int nivel);
int  equip_preco(int tipo, int nivel);
int  porao_capacidade(const Piloto *p);
int  porao_usado(const Piloto *p);
int  casco_maximo(const Piloto *p);
void estaleiro_tela(Jogo *j);
int  flag_tem(const Piloto *p, const char *nome);
void flag_por(Piloto *p, const char *nome);

/* voo.c */
Modo voo_rodar(Jogo *j);
void voo_chegar(Jogo *j, int perto_da_estacao);

/* estacao.c */
int  estacao_iniciar(void);
void estacao_encerrar(void);
Modo estacao_rodar(Jogo *j);

/* salvar.c */
int  jogo_salvar(const Jogo *j, const char *arquivo);
int  jogo_carregar(Jogo *j, const char *arquivo);

#endif
