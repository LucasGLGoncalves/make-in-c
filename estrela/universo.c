/*
 * universo.c — sistemas estelares, mercadorias, preços e saltos.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "dados.h"
#include "estrela.h"

Sistema SISTEMAS[MAX_SISTEMAS];
int     N_SISTEMAS = 0;

const char *const NOME_BEM[N_BENS] = {
    "Alimentos", "Água", "Minério", "Ligas metálicas",
    "Máquinas", "Eletrônicos", "Medicamentos", "Artigos de luxo"
};
static const int PRECO_BASE[N_BENS] = { 20, 12, 35, 60, 110, 160, 120, 300 };

/* Quanto cada economia paga por cada bem, em porcentagem do preço base.
 * Menos de 100: ali se produz (barato). Mais de 100: ali se consome. */
static const int MULT[N_ECONOMIAS][N_BENS] = {
 /*            ali  água min liga máq ele  med  lux */
 /* agrícola */ { 50,  60, 110, 110, 145, 120, 140, 110 },
 /* mineira  */ {150, 150,  45,  90, 135, 115, 130, 120 },
 /* indust.  */ {135, 120, 150,  55,  60, 110, 115, 110 },
 /* tecnol.  */ {125, 110, 110, 145, 110,  55,  60, 115 },
 /* rica     */ {110,  80, 120, 110, 110, 135, 120, 155 },
 /* fora-lei */ {120, 120, 100, 100, 135, 100, 165,  60 }
};
static const char *const NOME_ECONOMIA[N_ECONOMIAS] = {
    "agrícola", "mineradora", "industrial", "tecnológica", "rica", "fora da lei"
};

static int economia_por_id(const char *id)
{
    static const char *const IDS[N_ECONOMIAS] = {
        "agricola", "mineradora", "industrial", "tecnologica", "rica", "foradalei"
    };
    int i;
    for (i = 0; i < N_ECONOMIAS; i++)
        if (strcmp(id, IDS[i]) == 0) return i;
    return ECO_AGRICOLA;
}

int universo_carregar(void)
{
    char linha[512], *c[7];
    FILE *f = fopen("dados/sistemas.txt", "r");
    if (f == NULL) return 0;
    while (N_SISTEMAS < MAX_SISTEMAS && dados_ler_linha(f, linha, sizeof linha)) {
        Sistema *s = &SISTEMAS[N_SISTEMAS];
        if (dados_separar(linha, '|', c, 7) != 7) continue;
        snprintf(s->id, sizeof s->id, "%s", c[0]);
        snprintf(s->nome, sizeof s->nome, "%s", c[1]);
        s->x = atoi(c[2]);
        s->y = atoi(c[3]);
        s->economia = economia_por_id(c[4]);
        s->perigo = atoi(c[5]);
        snprintf(s->descricao, sizeof s->descricao, "%s", c[6]);
        N_SISTEMAS++;
    }
    fclose(f);
    return N_SISTEMAS > 1;
}

/* Preço de compra de hoje: base x economia x um sorteio de 85% a 115%. */
void universo_sortear_precos(int sistema)
{
    Sistema *s = &SISTEMAS[sistema];
    int b;
    for (b = 0; b < N_BENS; b++)
        s->preco[b] = PRECO_BASE[b] * MULT[s->economia][b] / 100
                    * rng_intervalo(85, 115) / 100;
}

/* A estação compra de você um pouco abaixo do preço que ela vende. */
int preco_venda(int sistema, int bem)
{
    return SISTEMAS[sistema].preco[bem] * 90 / 100;
}

/* Custo do salto em combustível (e em dias). As linhas da tela são
 * mais altas que as colunas são largas, por isso o dy conta dobrado. */
int distancia_salto(int a, int b)
{
    double dx = SISTEMAS[a].x - SISTEMAS[b].x;
    double dy = (SISTEMAS[a].y - SISTEMAS[b].y) * 2.0;
    return (int)ceil(sqrt(dx * dx + dy * dy) / 10.0);
}

static void desenhar_mapa_estelar(const Jogo *j, int escolhido)
{
    int i, atual = j->p.sistema;
    const Sistema *s = &SISTEMAS[escolhido];
    int custo = distancia_salto(atual, escolhido);
    char texto[300];

    tela_limpar();
    ui_janela(0, 0, TELA_L, 20, "Mapa estelar", AZUL_CLARO);
    for (i = 0; i < 60; i++)                 /* estrelas de fundo, sempre iguais */
        tela_por(1 + (i * 37) % 78, 1 + (i * 11) % 18, G_PONTO, CINZA_ESCURO, PRETO);
    for (i = 0; i < N_SISTEMAS; i++) {
        const Sistema *o = &SISTEMAS[i];
        int alcanca = distancia_salto(atual, i) <= j->p.combustivel;
        int cor = (i == atual) ? AMARELO : alcanca ? BRANCO : CINZA_ESCURO;
        int x = o->x + 3, y = o->y + 1;
        tela_por(x, y, i == atual ? G_SOL : G_BOLINHA, cor, PRETO);
        tela_por(x - 1, y, ' ', CINZA, PRETO);              /* um respiro em volta */
        tela_por(x + 1, y, ' ', CINZA, PRETO);
        if (x + 2 + utf8_largura(o->nome) < TELA_L - 1)
            tela_texto(x + 2, y, cor, PRETO, o->nome);       /* nome à direita  */
        else
            tela_texto(x - 2 - utf8_largura(o->nome), y, cor, PRETO, o->nome);
        if (i == escolhido) {
            tela_por(x - 1, y, '[', VERDE_CLARO, PRETO);
            tela_por(x + 1, y, ']', VERDE_CLARO, PRETO);
        }
    }

    tela_textof(1, 20, AMARELO, PRETO, "%s", s->nome);
    tela_textof(1 + utf8_largura(s->nome) + 2, 20, CINZA, PRETO,
                "economia %s, perigo %d/3", NOME_ECONOMIA[s->economia], s->perigo);
    ui_texto_quebrado(1, 21, 78, 2, CINZA, PRETO, s->descricao);
    if (escolhido == atual)
        snprintf(texto, sizeof texto, "Você está aqui.");
    else
        snprintf(texto, sizeof texto, "Salto: %d de combustível, %d dia(s). "
                 "Tanque: %d/%d.   Enter salta, Esc volta.",
                 custo, custo, j->p.combustivel, TANQUE);
    tela_texto(1, 23, custo > j->p.combustivel ? VERMELHO_CLARO : VERDE_CLARO,
               PRETO, texto);
}

int mapa_estelar(Jogo *j)
{
    int escolhido = j->p.sistema;
    for (;;) {
        int tecla;
        desenhar_mapa_estelar(j, escolhido);
        tela_mostrar();
        tecla = term_esperar_tecla();
        if (tecla == TECLA_DIREITA || tecla == TECLA_BAIXO)
            escolhido = (escolhido + 1) % N_SISTEMAS;
        else if (tecla == TECLA_ESQUERDA || tecla == TECLA_CIMA)
            escolhido = (escolhido + N_SISTEMAS - 1) % N_SISTEMAS;
        else if (tecla == TECLA_ESC)
            return -1;
        else if (tecla == TECLA_ENTER && escolhido != j->p.sistema &&
                 distancia_salto(j->p.sistema, escolhido) <= j->p.combustivel)
            return escolhido;
    }
}

static void animacao_salto(void)
{
    int quadro, i;
    for (quadro = 0; quadro < 24; quadro++) {
        tela_limpar();
        for (i = 0; i < 40; i++) {           /* riscos de luz correndo */
            int y = (i * 7) % TELA_A;
            int x = (i * 13 + quadro * (2 + i % 3) * 3) % TELA_L;
            int k;
            for (k = 0; k < quadro / 3 + 1; k++)
                tela_por(x - k, y, G_H, k == 0 ? BRANCO : AZUL_CLARO, PRETO);
        }
        tela_texto_centro(TELA_A / 2, BRANCO, PRETO, "  S A L T O  ");
        tela_mostrar();
        term_dormir(40);
    }
}

void saltar(Jogo *j, int destino)
{
    Piloto *p = &j->p;
    int custo = distancia_salto(p->sistema, destino), i;

    p->combustivel -= custo;
    p->dia += custo;
    p->sistema = destino;
    universo_sortear_precos(destino);
    animacao_salto();
    voo_chegar(j, 0);
    log_adicionar(&j->log, CIANO_CLARO, "Dia %d. Bem-vinda a %s.",
                  p->dia, SISTEMAS[destino].nome);

    for (i = p->n_missoes - 1; i >= 0; i--)        /* entregas atrasadas */
        if (p->dia > p->missoes[i].prazo) {
            log_adicionar(&j->log, VERMELHO_CLARO,
                          "O prazo de uma entrega para %s venceu. A carga foi confiscada.",
                          SISTEMAS[p->missoes[i].destino].nome);
            p->missoes[i] = p->missoes[--p->n_missoes];
        }
}
