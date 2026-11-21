/*
 * main.c — ESQUELETO de um jogo novo, para copiar e transformar.
 * Capítulo 33 de "Make in C: jogos de aventuras".
 *
 * Já funciona: título, mapa lido de arquivo, câmera, um herói que anda
 * por turnos, placas que podem ser lidas, um objetivo e um final.
 * Procure os comentários "TROQUE" e "ACRESCENTE": é por ali que o seu
 * jogo começa.
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "mapa.h"

#define NOME_DO_JOGO  "Meu Jogo"            /* TROQUE */
#define ARQUIVO_MAPA  "dados/mapa.txt"
#define VISTA_A       20                    /* linhas do mapa na tela */
#define MAX_PLACAS    16

enum { TL_CHAO, TL_PAREDE, TL_AGUA, TL_ARVORE, TL_SAIDA };

static const TileDef TILES[] = {   /* ACRESCENTE os seus tiles aqui */
    { G_PONTO,        CINZA_ESCURO, PRETO, 0,                  "chão"   },
    { G_BLOCO,        CINZA,        PRETO, T_SOLIDO | T_OPACO, "parede" },
    { G_APROX,        AZUL_CLARO,   AZUL,  T_SOLIDO,           "água"   },
    { G_PAUS,         VERDE,        PRETO, T_SOLIDO | T_OPACO, "árvore" },
    { G_TRIPLO,       PRETO,        AMARELO, 0,                "saída"  }
};
static const char LEGENDA[] = ".#~TE";      /* caractere i = tile i */

typedef struct {                   /* algo que se lê ao esbarrar */
    int  x, y;
    char texto[256];
} Placa;

typedef struct {
    Mapa    mapa;
    Camera  cam;
    int     x, y;                  /* o herói */
    long    turnos;
    Placa   placas[MAX_PLACAS];
    int     n_placas;
    Log     log;
    int     terminou;              /* 0 jogando, 1 venceu, 2 desistiu */
} Jogo;

/* Símbolos fora da legenda: '@' é o herói; dígitos são placas, cujo
 * texto vem nas linhas "1: texto" depois do mapa (lidas em carregar). */
static int simbolo(void *ctx, char c, int x, int y)
{
    Jogo *j = ctx;
    if (c == '@') { j->x = x; j->y = y; }
    else if (c >= '1' && c <= '9' && j->n_placas < MAX_PLACAS) {
        Placa *p = &j->placas[j->n_placas++];
        p->x = x; p->y = y;
        snprintf(p->texto, sizeof p->texto, "%c", c);   /* por enquanto, o dígito */
    }
    return TL_CHAO;
}

static int carregar(Jogo *j)
{
    char linha[256];
    int i;
    FILE *f = fopen(ARQUIVO_MAPA, "r");
    if (f == NULL) return 0;
    j->n_placas = 0;
    if (!mapa_ler(&j->mapa, f, TILES, LEGENDA, simbolo, j)) { fclose(f); return 0; }
    while (fgets(linha, sizeof linha, f) != NULL) {     /* "1: texto da placa" */
        linha[strcspn(linha, "\r\n")] = '\0';
        for (i = 0; i < j->n_placas; i++)
            if (j->placas[i].texto[0] == linha[0] && linha[1] == ':')
                snprintf(j->placas[i].texto, sizeof j->placas[i].texto, "%s", linha + 3);
    }
    fclose(f);
    j->cam.tela_x = 0;       j->cam.tela_y = 0;
    j->cam.largura = TELA_L; j->cam.altura = VISTA_A;
    return 1;
}

static int placa_em(const Jogo *j, int x, int y)
{
    int i;
    for (i = 0; i < j->n_placas; i++)
        if (j->placas[i].x == x && j->placas[i].y == y) return i;
    return -1;
}

static void desenhar(Jogo *j)
{
    int i, tx, ty;
    tela_limpar();
    camera_centralizar(&j->cam, &j->mapa, j->x, j->y);
    mapa_desenhar(&j->mapa, &j->cam, 0);
    for (i = 0; i < j->n_placas; i++)
        if (camera_na_tela(&j->cam, j->placas[i].x, j->placas[i].y, &tx, &ty))
            tela_por(tx, ty, '?', AMARELO, PRETO);
    if (camera_na_tela(&j->cam, j->x, j->y, &tx, &ty))
        tela_por(tx, ty, '@', BRANCO, PRETO);
    /* ACRESCENTE: monstros, itens, um painel... */
    tela_retangulo(0, VISTA_A, TELA_L, 1, ' ', PRETO, CINZA);
    tela_textof(1, VISTA_A, PRETO, CINZA, "%s   turno %ld", NOME_DO_JOGO, j->turnos);
    tela_texto(56, VISTA_A, AZUL, CINZA, "setas andar  Esc sair");
    log_desenhar(&j->log, 1, VISTA_A + 1, TELA_L - 2, TELA_A - VISTA_A - 1);
}

/* Devolve 1 se a ação gastou um turno. */
static int agir(Jogo *j, int tecla)
{
    int dx = 0, dy = 0, nx, ny, p;
    switch (tecla) {
        case TECLA_CIMA:     dy = -1; break;
        case TECLA_BAIXO:    dy = 1;  break;
        case TECLA_ESQUERDA: dx = -1; break;
        case TECLA_DIREITA:  dx = 1;  break;
        case TECLA_ESC:
            if (ui_confirmar("Sair do jogo?")) j->terminou = 2;
            return 0;
        default:
            return 0;                      /* ACRESCENTE outras teclas */
    }
    nx = j->x + dx;
    ny = j->y + dy;
    if ((p = placa_em(j, nx, ny)) >= 0) {
        ui_mensagem("Placa", j->placas[p].texto);
        return 0;
    }
    if (mapa_bloqueia(&j->mapa, nx, ny)) return 0;
    j->x = nx;
    j->y = ny;
    if (mapa_tile(&j->mapa, nx, ny) == TL_SAIDA) j->terminou = 1;   /* TROQUE */
    return 1;
}

static void jogar(Jogo *j)
{
    while (!j->terminou) {
        desenhar(j);
        tela_mostrar();
        if (agir(j, term_esperar_tecla())) {
            j->turnos++;
            /* ACRESCENTE: o mundo reage (monstros, relógio, eventos...) */
        }
    }
}

int main(void)
{
    static Jogo jogo;
    const char *opcoes[] = { "Jogar", "Sair" };
    int colunas, linhas;

    term_tamanho(&colunas, &linhas);
    if (colunas < TELA_L || linhas < TELA_A) {
        printf("Aumente o terminal para %dx%d.\n", TELA_L, TELA_A);
        return 1;
    }
    term_iniciar();
    rng_semear(rng_semente_do_relogio());

    for (;;) {
        tela_limpar();
        tela_texto_centro(5, AMARELO, PRETO, NOME_DO_JOGO);          /* TROQUE */
        tela_texto_centro(7, CINZA, PRETO, "um jogo feito em C");
        tela_mostrar();
        if (ui_menu(NOME_DO_JOGO, opcoes, 2, 0) != 0) break;

        memset(&jogo, 0, sizeof jogo);         /* mapa NULL: começa do zero */
        if (!carregar(&jogo)) {
            ui_mensagem("Erro", "Não achei " ARQUIVO_MAPA ".");
            continue;
        }
        log_adicionar(&jogo.log, CINZA, "Encontre a saída. Esbarre nas placas para lê-las.");
        jogar(&jogo);
        if (jogo.terminou == 1) {
            char texto[96];
            snprintf(texto, sizeof texto, "Você chegou à saída em %ld turnos.", jogo.turnos);
            ui_mensagem("Fim", texto);
        }
        mapa_destruir(&jogo.mapa);
    }
    term_encerrar();
    return 0;
}
