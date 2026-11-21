/*
 * coroa.h — "A Coroa de Ferro": tipos e funções compartilhadas.
 */
#ifndef COROA_H
#define COROA_H

#include "mapa.h"
#include "ui.h"

#define VISTA_L      60     /* janela do mapa na tela */
#define VISTA_A      18
#define PAINEL_X     60
#define LOG_Y        18
#define MASMORRA_L   70     /* tamanho dos níveis gerados */
#define MASMORRA_A   34
#define PROFUNDIDADE_FINAL 5
#define MAX_ATORES   64
#define MAX_CHAO     128
#define MAX_MOCHILA  18
#define MAX_ITEM_DEFS 40
#define MAX_MONSTRO_DEFS 16
#define MAX_FLAGS    24

enum {
    TL_ROCHA, TL_PAREDE, TL_CHAO, TL_PORTA, TL_DESCE, TL_SOBE,
    TL_GRAMA, TL_ARVORE, TL_AGUA, TL_ESTRADA, TL_MURO, TL_ASSOALHO,
    TL_BALCAO, TL_POCO
};

typedef enum { IT_OURO, IT_ARMA, IT_ARMADURA, IT_POCAO,
               IT_PERGAMINHO, IT_COROA } ItemTipo;
typedef enum { SLOT_NENHUM, SLOT_MAO, SLOT_ESCUDO, SLOT_CORPO,
               SLOT_CABECA, SLOT_ANEL, N_SLOTS } Slot;

typedef struct {
    char         id[24];
    char         nome[40];
    unsigned int glifo;
    int          cor;
    int          tipo;          /* ItemTipo */
    int          slot;          /* Slot     */
    int          valor;         /* preço em ouro */
    int          ataque, defesa;
    int          efeito;        /* cura, quantidade de ouro etc. */
    int          prof_min;      /* 0 = nunca aparece sozinho na masmorra */
    char         descricao[120];
} ItemDef;

typedef struct {
    char         id[24];
    char         nome[32];
    unsigned int glifo;
    int          cor;
    int          hp, ataque, defesa, xp;
    int          prof_min, prof_max;
    int          erratico;      /* 1 = anda meio sem rumo (morcegos) */
} MonstroDef;

typedef struct { int def, qtd, equipado; } ItemPilha;
typedef struct { int def, qtd, x, y; } ItemChao;

typedef struct {
    int raca;          /* índice em MonstroDef, ou -1 se for NPC   */
    int npc;           /* índice na tabela de NPCs, ou -1          */
    int x, y, hp, hp_max;
    int vivo, acordado;
} Ator;

typedef struct {
    char      nome[24];
    int       x, y, hp, hp_max;
    int       ataque_base, defesa_base;
    int       nivel, xp, ouro;
    ItemPilha mochila[MAX_MOCHILA];
    int       n_mochila;
    long      turnos;
    int       abatidos;
    int       prof_max;          /* o mais fundo que já chegou */
} Heroi;

typedef enum { JOGANDO, FIM_MORTE, FIM_VITORIA, FIM_SALVOU } Fim;

typedef struct {
    Mapa     mapa;
    Camera   cam;
    int      profundidade;      /* 0 = aldeia */
    Ator     atores[MAX_ATORES];
    int      n_atores;
    ItemChao chao[MAX_CHAO];
    int      n_chao;
    Heroi    heroi;
    Log      log;
    char     flags[MAX_FLAGS][24];
    int      n_flags;
    Fim      fim;
    char     causa_morte[40];   /* quem derrubou o herói */
    int     *dist;              /* mapa de distâncias até o herói */
} Jogo;

/* dados_jogo.c */
extern ItemDef    ITENS[MAX_ITEM_DEFS];
extern int        N_ITENS;
extern MonstroDef MONSTROS[MAX_MONSTRO_DEFS];
extern int        N_MONSTROS;
int  dados_carregar_tudo(void);
int  item_por_id(const char *id);

/* mundo.c */
extern const TileDef TILES[];
int  mundo_ir_para(Jogo *j, int profundidade, int vindo_de_cima);
int  ator_em(const Jogo *j, int x, int y);
int  chao_em(const Jogo *j, int x, int y);
void chao_por(Jogo *j, int def, int qtd, int x, int y);

/* heroi.c */
void heroi_criar(Heroi *h, const char *nome);
int  heroi_ataque(const Heroi *h);
int  heroi_defesa(const Heroi *h);
int  heroi_acao(Jogo *j, int tecla);     /* 1 se gastou um turno */
int  mochila_adicionar(Heroi *h, int def, int qtd);
void mochila_remover(Heroi *h, int indice, int qtd);
int  mochila_procurar(const Heroi *h, const char *id);
int  slot_ocupado(const Heroi *h, int slot);
int  inventario_tela(Jogo *j);    /* 1 se gastou um turno */
void heroi_ganhar_xp(Jogo *j, int xp);

/* monstros.c */
int  combate_rolar(int ataque, int defesa, int *dano);
void monstros_turno(Jogo *j);
void heroi_atacar(Jogo *j, int indice_ator);

/* aldeia.c */
void aldeia_conversar(Jogo *j, int indice_ator);
int  flag_tem(const Jogo *j, const char *nome);
void flag_por(Jogo *j, const char *nome);
unsigned int npc_glifo(int npc);
int  npc_cor(int npc);
const char *npc_nome(int npc);
int  npc_por_simbolo(char c);
int  aldeia_iniciar(void);
void aldeia_encerrar(void);

/* desenho.c */
void desenhar_jogo(Jogo *j);

/* salvar.c */
int  jogo_salvar(const Jogo *j, const char *arquivo);
int  jogo_carregar(Jogo *j, const char *arquivo);

#endif
