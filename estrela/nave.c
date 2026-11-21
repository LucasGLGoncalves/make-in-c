/*
 * nave.c — o equipamento da Estrela Errante e o estaleiro.
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "estrela.h"

typedef struct {
    const char *nome;
    int         preco;
    int         valor;    /* dano, escudo máximo, empuxo x100, toneladas, casco */
} PecaDef;

static const PecaDef PECAS[N_EQUIP][3] = {
    { { "Laser de mineração",   0,    2 },
      { "Laser militar II",     2500, 3 },
      { "Canhão de plasma",     7000, 5 } },
    { { "Escudo civil",         0,    20 },
      { "Escudo reforçado",     3000, 40 },
      { "Escudo militar",       8000, 70 } },
    { { "Motor de carga",       0,    100 },
      { "Motor de corrida",     2000, 140 },
      { "Motor de fusão",       6000, 185 } },
    { { "Porão de 20 t",        0,    20 },
      { "Porão de 35 t",        2500, 35 },
      { "Porão de 50 t",        6000, 50 } },
    { { "Casco de alumínio",    0,    60 },
      { "Casco de titânio",     2500, 100 },
      { "Casco de neutrônio",   7000, 160 } }
};
static const char *const NOME_EQUIP[N_EQUIP] = {
    "Arma", "Escudo", "Motor", "Porão", "Casco"
};

const char *equip_nome(int tipo, int nivel)  { return PECAS[tipo][nivel].nome;  }
int         equip_valor(int tipo, int nivel) { return PECAS[tipo][nivel].valor; }
int         equip_preco(int tipo, int nivel) { return PECAS[tipo][nivel].preco; }

int porao_capacidade(const Piloto *p) { return equip_valor(EQ_PORAO, p->equip[EQ_PORAO]); }
int casco_maximo(const Piloto *p)     { return equip_valor(EQ_CASCO, p->equip[EQ_CASCO]); }

int porao_usado(const Piloto *p)
{
    int total = 0, i;
    for (i = 0; i < N_BENS; i++) total += p->carga[i];
    for (i = 0; i < p->n_missoes; i++) total += p->missoes[i].qtd;  /* caixas lacradas */
    return total;
}

int flag_tem(const Piloto *p, const char *nome)
{
    int i;
    for (i = 0; i < p->n_flags; i++)
        if (strcmp(p->flags[i], nome) == 0) return 1;
    return 0;
}

void flag_por(Piloto *p, const char *nome)
{
    if (flag_tem(p, nome) || p->n_flags >= MAX_FLAGS) return;
    snprintf(p->flags[p->n_flags++], sizeof p->flags[0], "%s", nome);
}

/* A "planta" da nave: o mesmo truque do boneco de equipamento do
 * capítulo 23, agora com uma nave no lugar de um corpo. */
static void desenhar_caixa(int x, int y, int tipo, const Piloto *p)
{
    tela_moldura(x, y, 24, 3, 0, BRANCO, PRETO);
    tela_textof(x + 2, y, AMARELO, PRETO, "%d %s", tipo + 1, NOME_EQUIP[tipo]);
    tela_texto_max(x + 2, y + 1, CINZA, PRETO, equip_nome(tipo, p->equip[tipo]), 20);
}

static void desenhar_planta(const Piloto *p)
{
    tela_por(17, 4, G_TRI_CIMA, CIANO_CLARO, PRETO);           /* nariz      */
    tela_retangulo(16, 5, 3, 2, G_BLOCO, CINZA, PRETO);
    tela_retangulo(14, 7, 7, 4, G_BLOCO, CINZA, PRETO);         /* corpo      */
    tela_retangulo(8, 9, 6, 2, G_BLOCO, CINZA_ESCURO, PRETO);   /* asa esq.   */
    tela_retangulo(21, 9, 6, 2, G_BLOCO, CINZA_ESCURO, PRETO);  /* asa dir.   */
    tela_retangulo(15, 11, 5, 2, G_BLOCO, CINZA, PRETO);
    tela_por(15, 13, G_TRI_BAIXO, VERMELHO_CLARO, PRETO);       /* motores    */
    tela_por(19, 13, G_TRI_BAIXO, VERMELHO_CLARO, PRETO);
    tela_por(17, 8, G_QUADRADO, AZUL_CLARO, CINZA);             /* cabine     */

    desenhar_caixa(6, 0, EQ_ARMA, p);
    tela_por(17, 3, G_V, VERDE_CLARO, PRETO);
    desenhar_caixa(0, 15, EQ_ESCUDO, p);
    tela_por(9, 12, G_V, VERDE_CLARO, PRETO);
    tela_por(9, 13, G_V, VERDE_CLARO, PRETO);
    tela_por(9, 14, G_V, VERDE_CLARO, PRETO);
    desenhar_caixa(0, 19, EQ_MOTOR, p);
    desenhar_caixa(26, 15, EQ_PORAO, p);
    tela_por(25, 12, G_V, VERDE_CLARO, PRETO);
    tela_por(25, 13, G_V, VERDE_CLARO, PRETO);
    tela_por(25, 14, G_V, VERDE_CLARO, PRETO);
    desenhar_caixa(26, 19, EQ_CASCO, p);
}

static int vende_nivel(int sistema, int nivel)
{
    int eco = SISTEMAS[sistema].economia;
    if (nivel <= 1) return eco != ECO_AGRICOLA;               /* nível 1: quase todo lugar */
    return eco == ECO_INDUSTRIAL || eco == ECO_TECNOLOGICA;   /* nível 2: só os fabricantes */
}

void estaleiro_tela(Jogo *j)
{
    Piloto *p = &j->p;
    for (;;) {
        int t, y = 2, tecla;
        int reparo = (casco_maximo(p) - p->casco) * 15;
        int abastecer = (TANQUE - p->combustivel) * 40;

        tela_limpar();
        desenhar_planta(p);
        ui_janela(51, 0, 29, TELA_A, "Estaleiro", AMARELO);
        tela_textof(53, 1, AMARELO, PRETO, "Créditos: %ld", p->creditos);
        for (t = 0; t < N_EQUIP; t++) {
            int prox = p->equip[t] + 1;
            tela_textof(53, y, BRANCO, PRETO, "%d %s", t + 1, NOME_EQUIP[t]);
            if (prox > 2)
                tela_texto(55, y + 1, CINZA_ESCURO, PRETO, "(no máximo)");
            else if (!vende_nivel(p->sistema, prox))
                tela_texto(55, y + 1, CINZA_ESCURO, PRETO, "(não vendem aqui)");
            else {
                tela_texto_max(55, y + 1, CINZA, PRETO, equip_nome(t, prox), 17);
                tela_textof(73, y + 1, VERDE_CLARO, PRETO, "%4d", equip_preco(t, prox));
            }
            y += 3;
        }
        tela_textof(53, 17, CIANO_CLARO, PRETO, "R reparar %d/%d", p->casco, casco_maximo(p));
        tela_textof(55, 18, CINZA, PRETO, "%d cr", reparo);
        tela_textof(53, 19, CIANO_CLARO, PRETO, "A abastecer %d/%d", p->combustivel, TANQUE);
        tela_textof(55, 20, CINZA, PRETO, "%d cr", abastecer);
        tela_texto(53, 22, CINZA_ESCURO, PRETO, "Esc: sair");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == TECLA_ESC) return;
        if ((tecla == 'r' || tecla == 'R') && reparo > 0) {
            if (p->creditos < reparo) ui_mensagem("Estaleiro", "Créditos insuficientes.");
            else { p->creditos -= reparo; p->casco = casco_maximo(p); }
        }
        if ((tecla == 'a' || tecla == 'A') && abastecer > 0) {
            if (p->creditos < abastecer) ui_mensagem("Estaleiro", "Créditos insuficientes.");
            else { p->creditos -= abastecer; p->combustivel = TANQUE; }
        }
        t = tecla - '1';
        if (t >= 0 && t < N_EQUIP && p->equip[t] < 2 &&
            vende_nivel(p->sistema, p->equip[t] + 1)) {
            int prox = p->equip[t] + 1;
            char pergunta[128];
            snprintf(pergunta, sizeof pergunta, "Instalar %s por %d créditos?",
                     equip_nome(t, prox), equip_preco(t, prox));
            if (p->creditos < equip_preco(t, prox))
                ui_mensagem("Estaleiro", "Créditos insuficientes.");
            else if (ui_confirmar(pergunta)) {
                int casco_antes = casco_maximo(p);
                p->creditos -= equip_preco(t, prox);
                p->equip[t] = prox;
                if (t == EQ_CASCO) p->casco += casco_maximo(p) - casco_antes;
                log_adicionar(&j->log, VERDE_CLARO, "Instalado: %s.", equip_nome(t, prox));
            }
        }
    }
}
