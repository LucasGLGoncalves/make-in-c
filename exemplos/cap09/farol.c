/*
 * farol.c — "O Farol Abandonado", uma aventura de texto completa.
 * Capítulo 9: structs, enums e tabelas de dados.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum { NORTE, SUL, LESTE, OESTE, CIMA, BAIXO, N_DIRECOES } Direcao;

#define SEM_SAIDA   -1
#define NA_MOCHILA  -1
#define SUMIU       -2

typedef struct {
    const char *nome;
    const char *descricao;
    int         saidas[N_DIRECOES];   /* número da sala vizinha, ou SEM_SAIDA */
} Sala;

typedef struct {
    const char *nome;
    const char *descricao;
    int         local;                /* sala onde está, ou NA_MOCHILA */
} Item;

typedef struct {
    int sala;
    int porta_aberta;
    int lampada_cheia;
    int venceu;
} Estado;

enum { PRAIA, PORTA, TERREO, ESCADA, TOPO, DESPENSA, BARCO, N_SALAS };

static Sala salas[N_SALAS] = {
    /*            nome                descrição                                  N  S  L  O  C  B */
    { "Praia",    "Uma praia de pedras molhadas. Ao norte, um farol branco e apagado. "
                  "A leste, um barco virado na areia.",
                  { PORTA, -1, BARCO, -1, -1, -1 } },
    { "Porta do farol", "Uma porta de ferro, cheia de ferrugem.",
                  { -1, PRAIA, -1, -1, -1, -1 } },        /* norte abre com a chave */
    { "Térreo",   "Uma escada em espiral sobe no escuro. Há uma despensa a oeste.",
                  { -1, PORTA, -1, DESPENSA, ESCADA, -1 } },
    { "Escada",   "Os degraus rangem. O vento assobia pelas janelas quebradas.",
                  { -1, -1, -1, -1, TOPO, TERREO } },
    { "Topo do farol", "A grande lente do farol. A mecha da lâmpada está seca. "
                  "Ao longe, um navio navega direto para as rochas!",
                  { -1, -1, -1, -1, -1, ESCADA } },
    { "Despensa", "Prateleiras vazias e teias de aranha.",
                  { -1, -1, TERREO, -1, -1, -1 } },
    { "Barco virado", "Um velho barco de pesca, de quilha para cima.",
                  { -1, -1, -1, PRAIA, -1, -1 } }
};

static Item itens[] = {
    { "chave",    "Uma chave grande e enferrujada.",          BARCO    },
    { "oleo",     "Uma lata de óleo de lamparina, cheia.",    DESPENSA },
    { "fosforos", "Uma caixa de fósforos. Ainda secos!",      ESCADA   },
    { "concha",   "Uma concha bonita. Não serve para nada.",  PRAIA    }
};
#define N_ITENS ((int)(sizeof itens / sizeof itens[0]))

static const char *const NOME_DIRECAO[N_DIRECOES] = {
    "norte", "sul", "leste", "oeste", "cima", "baixo"
};

static int item_por_nome(const char *nome)
{
    int i;
    for (i = 0; i < N_ITENS; i++)
        if (strcmp(itens[i].nome, nome) == 0) return i;
    return -1;
}

static void descrever(const Estado *e)
{
    const Sala *s = &salas[e->sala];
    int i, d, achou = 0;
    printf("\n== %s ==\n%s\n", s->nome, s->descricao);
    for (i = 0; i < N_ITENS; i++)
        if (itens[i].local == e->sala) printf("Aqui há: %s.\n", itens[i].nome);
    printf("Saídas:");
    for (d = 0; d < N_DIRECOES; d++)
        if (s->saidas[d] != SEM_SAIDA) { printf(" %s", NOME_DIRECAO[d]); achou = 1; }
    if (e->sala == PORTA && !e->porta_aberta) printf(" (a porta ao norte está trancada)");
    printf(achou ? "\n" : " nenhuma\n");
}

static void ir(Estado *e, const char *para)
{
    int d;
    for (d = 0; d < N_DIRECOES; d++)
        if (strcmp(para, NOME_DIRECAO[d]) == 0 || para[0] == NOME_DIRECAO[d][0])
            break;
    if (d == N_DIRECOES) { printf("Ir para onde?\n"); return; }
    if (salas[e->sala].saidas[d] == SEM_SAIDA) {
        printf("Não dá para ir para %s.\n", NOME_DIRECAO[d]);
        return;
    }
    e->sala = salas[e->sala].saidas[d];
    descrever(e);
}

static void pegar(Estado *e, const char *nome)
{
    int i = item_por_nome(nome);
    if (i < 0 || itens[i].local != e->sala) { printf("Não vejo isso aqui.\n"); return; }
    itens[i].local = NA_MOCHILA;
    printf("Você pega: %s.\n", nome);
}

static void usar(Estado *e, const char *nome)
{
    int i = item_por_nome(nome);
    if (i < 0 || itens[i].local != NA_MOCHILA) { printf("Você não tem isso.\n"); return; }
    if (strcmp(nome, "chave") == 0 && e->sala == PORTA) {
        e->porta_aberta = 1;
        salas[PORTA].saidas[NORTE] = TERREO;        /* a sala muda! */
        printf("A chave gira com um rangido. A porta se abre.\n");
    } else if (strcmp(nome, "oleo") == 0 && e->sala == TOPO) {
        e->lampada_cheia = 1;
        itens[i].local = SUMIU;
        printf("Você enche a lâmpada com o óleo. A mecha fica encharcada.\n");
    } else if (strcmp(nome, "fosforos") == 0 && e->sala == TOPO) {
        if (!e->lampada_cheia) {
            printf("O fósforo queima a mecha seca, que vira cinza. Falta alguma coisa.\n");
        } else {
            printf("A chama sobe, a lente gira e um facho de luz corta a noite.\n"
                   "Lá no mar, o navio desvia das rochas no último instante.\n");
            e->venceu = 1;
        }
    } else {
        printf("Nada acontece.\n");
    }
}

static void inventario(void)
{
    int i, tem = 0;
    printf("Na mochila:");
    for (i = 0; i < N_ITENS; i++)
        if (itens[i].local == NA_MOCHILA) { printf(" %s", itens[i].nome); tem = 1; }
    printf(tem ? "\n" : " nada\n");
}

int main(void)
{
    Estado e = { PRAIA, 0, 0, 0 };
    char linha[100], verbo[30], objeto[30];

    printf("O FAROL ABANDONADO\nComandos: ir <direção>, n/s/l/o/c/b, olhar, pegar <coisa>,\n"
           "usar <coisa>, examinar <coisa>, inventario, sair.\n");
    descrever(&e);

    while (!e.venceu) {
        int i;
        printf("> ");
        if (fgets(linha, sizeof linha, stdin) == NULL) break;
        for (i = 0; linha[i]; i++) linha[i] = (char)tolower((unsigned char)linha[i]);
        verbo[0] = objeto[0] = '\0';
        sscanf(linha, "%29s %29s", verbo, objeto);

        if (strcmp(verbo, "sair") == 0) break;
        else if (strcmp(verbo, "ir") == 0) ir(&e, objeto);
        else if (strlen(verbo) == 1 && strchr("nslocb", verbo[0])) ir(&e, verbo);
        else if (strcmp(verbo, "olhar") == 0) descrever(&e);
        else if (strcmp(verbo, "pegar") == 0) pegar(&e, objeto);
        else if (strcmp(verbo, "usar") == 0) usar(&e, objeto);
        else if (strcmp(verbo, "inventario") == 0 || strcmp(verbo, "i") == 0) inventario();
        else if (strcmp(verbo, "examinar") == 0) {
            int k = item_por_nome(objeto);
            if (k >= 0 && (itens[k].local == NA_MOCHILA || itens[k].local == e.sala))
                printf("%s\n", itens[k].descricao);
            else
                printf("Não vejo isso.\n");
        } else if (verbo[0] != '\0') printf("Não entendi \"%s\".\n", verbo);
    }
    if (e.venceu) printf("\n*** Você salvou o navio! FIM ***\n");
    return 0;
}
