/*
 * estacao.c — a estação a pé: pessoas, mercado, missões e banco.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "dados.h"
#include "dialogo.h"
#include "estrela.h"

#define MAX_PESSOAS 12

static const TileDef TILES_ESTACAO[] = {
    { ' ',            PRETO,        PRETO, T_SOLIDO | T_OPACO, "vazio"   },
    { G_BLOCO,        CINZA,        PRETO, T_SOLIDO | T_OPACO, "parede"  },
    { G_PONTO,        CINZA_ESCURO, PRETO, 0,                  "piso"    },
    { G_SOMBRA_LEVE,  MAGENTA,      PRETO, 0,                  "carpete" },
    { G_SOMBRA_MEDIA, CIANO,        PRETO, 0,                  "porta"   },
    { G_PI,           CINZA,        PRETO, T_SOLIDO,           "balcão"  },
    { G_QUADRADO,     MARROM,       PRETO, T_SOLIDO,           "caixote" },
    { G_TRI_CIMA,     BRANCO,       AZUL,  0,                  "sua nave"},
    { G_SOMBRA_LEVE,  AZUL,         PRETO, T_SOLIDO,           "janela"  },
    { G_TRIPLO,       CINZA_ESCURO, PRETO, 0,                  "hangar"  }
};
enum { E_VAZIO, E_PAREDE, E_PISO, E_CARPETE, E_PORTA, E_BALCAO, E_CAIXOTE,
       E_NAVE, E_JANELA, E_HANGAR };
static const char LEGENDA[] = " #.,+|MSW=";

typedef struct {
    char letra, nome[48], no[32], condicao[48];
    unsigned int glifo;
    int cor, x, y;
} Pessoa;

static Pessoa   pessoas[MAX_PESSOAS];
static int      n_pessoas;
static Dialogos dialogos;
static int      inicio_x, inicio_y;

static int cor_por_nome(const char *nome)
{
    static const char *const CORES[16] = {
        "PRETO", "AZUL", "VERDE", "CIANO", "VERMELHO", "MAGENTA", "MARROM",
        "CINZA", "CINZA_ESCURO", "AZUL_CLARO", "VERDE_CLARO", "CIANO_CLARO",
        "VERMELHO_CLARO", "MAGENTA_CLARO", "AMARELO", "BRANCO"
    };
    int i;
    for (i = 0; i < 16; i++)
        if (strcmp(nome, CORES[i]) == 0) return i;
    return CINZA;
}

static int simbolo(void *ctx, char c, int x, int y)
{
    int i;
    (void)ctx;
    if (c == '@') { inicio_x = x; inicio_y = y; return E_HANGAR; }
    for (i = 0; i < n_pessoas; i++)
        if (pessoas[i].letra == c) { pessoas[i].x = x; pessoas[i].y = y; }
    return E_PISO;
}

static int carregar_estacao(Jogo *j)
{
    char linha[256], *c[6];
    FILE *f = fopen("dados/estacao.txt", "r");
    int ok;
    if (f == NULL) return 0;
    n_pessoas = 0;
    while (dados_ler_linha(f, linha, sizeof linha) && strcmp(linha, "MAPA") != 0) {
        Pessoa *p = &pessoas[n_pessoas];
        int n = dados_separar(linha, '|', c, 6);
        if (n < 5 || n_pessoas >= MAX_PESSOAS) continue;
        p->letra = c[0][0];
        snprintf(p->nome, sizeof p->nome, "%s", c[1]);
        p->glifo = (c[2][0] == 'U') ? (unsigned int)strtoul(c[2] + 2, NULL, 16)
                                    : (unsigned char)c[2][0];
        p->cor = cor_por_nome(c[3]);
        snprintf(p->no, sizeof p->no, "%s", c[4]);
        snprintf(p->condicao, sizeof p->condicao, "%s", n > 5 ? c[5] : "");
        p->x = p->y = -1;
        n_pessoas++;
    }
    mapa_destruir(&j->estacao);
    ok = mapa_ler(&j->estacao, f, TILES_ESTACAO, LEGENDA, simbolo, j);
    fclose(f);
    j->ex = inicio_x;
    j->ey = inicio_y;
    j->cam.tela_x = 0;  j->cam.tela_y = 0;
    j->cam.largura = 60; j->cam.altura = 18;
    return ok;
}

int estacao_iniciar(void)
{
    return dialogos_carregar(&dialogos, "dados/dialogos.txt");
}

void estacao_encerrar(void)
{
    dialogos_liberar(&dialogos);
}

static void mercado_tela(Jogo *j)
{
    Piloto *p = &j->p;
    int sel = 0;
    for (;;) {
        int b, tecla, livre = porao_capacidade(p) - porao_usado(p);
        char nome[64];
        tela_limpar();
        ui_janela(4, 1, 72, 21, "Mercado", VERDE);
        tela_textof(7, 2, AMARELO, PRETO, "%s", SISTEMAS[p->sistema].nome);
        tela_textof(40, 2, CINZA, PRETO, "Créditos %ld   Porão %d/%d",
                    p->creditos, porao_usado(p), porao_capacidade(p));
        tela_texto(7, 4, CINZA_ESCURO, PRETO,
                   "Mercadoria             Compra    Venda    No porão");
        for (b = 0; b < N_BENS; b++) {
            int y = 6 + b * 2, fundo = (b == sel) ? AZUL : PRETO;
            tela_retangulo(5, y, 70, 1, ' ', CINZA, fundo);
            utf8_ajustar(nome, sizeof nome, NOME_BEM[b], 20);
            tela_texto(7, y, BRANCO, fundo, nome);
            tela_textof(30, y, VERDE_CLARO, fundo, "%5d", SISTEMAS[p->sistema].preco[b]);
            tela_textof(39, y, AMARELO, fundo, "%5d", preco_venda(p->sistema, b));
            tela_textof(50, y, CINZA, fundo, "%5d t", p->carga[b]);
        }
        tela_texto(7, 23 - 1, CINZA_ESCURO, PRETO,
                   "↑↓ escolher  → compra 1  ← vende 1  C compra tudo  V vende tudo  Esc sai");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == TECLA_ESC) return;
        if (tecla == TECLA_CIMA)  sel = (sel + N_BENS - 1) % N_BENS;
        if (tecla == TECLA_BAIXO) sel = (sel + 1) % N_BENS;
        if ((tecla == TECLA_DIREITA || tecla == 'C' || tecla == 'c') && livre > 0) {
            int preco = SISTEMAS[p->sistema].preco[sel];
            int qtd = (tecla == TECLA_DIREITA) ? 1 : livre;
            if (qtd * preco > p->creditos) qtd = (int)(p->creditos / preco);
            p->creditos -= (long)qtd * preco;
            p->carga[sel] += qtd;
        }
        if ((tecla == TECLA_ESQUERDA || tecla == 'V' || tecla == 'v') && p->carga[sel] > 0) {
            int qtd = (tecla == TECLA_ESQUERDA) ? 1 : p->carga[sel];
            p->creditos += (long)qtd * preco_venda(p->sistema, sel);
            p->carga[sel] -= qtd;
        }
    }
}

static void entregar_missoes(Jogo *j)
{
    Piloto *p = &j->p;
    int i;
    for (i = p->n_missoes - 1; i >= 0; i--) {
        Missao *m = &p->missoes[i];
        if (m->destino != p->sistema) continue;
        p->creditos += m->pagamento;
        log_adicionar(&j->log, VERDE_CLARO, "Entrega concluída: %d caixas de %s. +%d créditos.",
                      m->qtd, NOME_BEM[m->bem], m->pagamento);
        *m = p->missoes[--p->n_missoes];      /* tira da lista */
    }
}

static void sortear_oferta(Jogo *j)
{
    Missao *o = &j->oferta;
    int dias;
    j->tem_oferta = 0;
    if (j->p.n_missoes >= MAX_MISSOES) return;
    do {
        o->destino = rng_intervalo(0, N_SISTEMAS - 1);
    } while (o->destino == j->p.sistema);
    dias = distancia_salto(j->p.sistema, o->destino);
    o->bem = rng_intervalo(0, N_BENS - 1);
    o->qtd = rng_intervalo(3, 8);
    o->pagamento = dias * 180 + o->qtd * 45 + rng_intervalo(0, 10) * 20;
    o->prazo = j->p.dia + dias * 2 + 6;
    j->tem_oferta = 1;
}

static void mostrar_oferta(Jogo *j)
{
    Missao *o = &j->oferta;
    char texto[300];
    snprintf(texto, sizeof texto,
             "Levar %d caixas lacradas de %s até %s.\n\nPagamento: %d créditos "
             "na entrega.\nPrazo: até o dia %d (hoje é dia %d).\n\nAceita?",
             o->qtd, NOME_BEM[o->bem], SISTEMAS[o->destino].nome,
             o->pagamento, o->prazo, j->p.dia);
    if (!ui_confirmar(texto)) return;
    if (porao_capacidade(&j->p) - porao_usado(&j->p) < o->qtd) {
        ui_mensagem("Agente", "\"Seu porão não comporta essa carga, capitã.\"");
        return;
    }
    j->p.missoes[j->p.n_missoes++] = *o;
    j->tem_oferta = 0;
    log_adicionar(&j->log, AMARELO, "Missão aceita: entregar em %s até o dia %d.",
                  SISTEMAS[o->destino].nome, o->prazo);
}

static void porao_tela(const Jogo *j)
{
    const Piloto *p = &j->p;
    char texto[900] = "", linha[160];
    int b, i;
    for (b = 0; b < N_BENS; b++)
        if (p->carga[b] > 0) {
            snprintf(linha, sizeof linha, "%3d t  %s\n", p->carga[b], NOME_BEM[b]);
            strcat(texto, linha);
        }
    if (texto[0] == '\0') strcat(texto, "(nenhuma mercadoria)\n");
    strcat(texto, "\nEntregas:\n");
    for (i = 0; i < p->n_missoes; i++) {
        snprintf(linha, sizeof linha, "- %d cx de %s para %s, até o dia %d\n",
                 p->missoes[i].qtd, NOME_BEM[p->missoes[i].bem],
                 SISTEMAS[p->missoes[i].destino].nome, p->missoes[i].prazo);
        strcat(texto, linha);
    }
    if (p->n_missoes == 0) strcat(texto, "(nenhuma)\n");
    ui_mensagem("Porão e entregas", texto);
}

static int condicao(void *ctx, const char *c)
{
    Jogo *j = ctx;
    if (texto_comeca(c, "sistema:"))  return strcmp(SISTEMAS[j->p.sistema].id, c + 8) == 0;
    if (texto_comeca(c, "creditos:")) return j->p.creditos >= atol(c + 9);
    if (texto_comeca(c, "flag:"))     return flag_tem(&j->p, c + 5);
    if (strcmp(c, "divida") == 0)     return j->p.divida > 0;
    if (strcmp(c, "oferta") == 0)     return j->tem_oferta;
    if (strcmp(c, "pode_quitar") == 0) return j->p.creditos >= j->p.divida;
    return 0;
}

static void pagar(Jogo *j, long valor)
{
    if (valor > j->p.divida) valor = j->p.divida;
    if (valor > j->p.creditos) valor = j->p.creditos;
    j->p.creditos -= valor;
    j->p.divida -= valor;
    log_adicionar(&j->log, AMARELO, "Pagamento de %ld ao Banco Orbital. Dívida: %ld.",
                  valor, j->p.divida);
    if (j->p.divida == 0) j->fim = FIM_VITORIA;
}

static void efeito(void *ctx, const char *cmd)
{
    Jogo *j = ctx;
    if (strcmp(cmd, "mercado") == 0)          mercado_tela(j);
    else if (strcmp(cmd, "estaleiro") == 0)   estaleiro_tela(j);
    else if (strcmp(cmd, "oferta") == 0)      mostrar_oferta(j);
    else if (texto_comeca(cmd, "pagar "))     pagar(j, atol(cmd + 6));
    else if (strcmp(cmd, "quitar") == 0)      pagar(j, j->p.divida);
    else if (texto_comeca(cmd, "creditos "))  j->p.creditos += atol(cmd + 9);
    else if (texto_comeca(cmd, "flag "))      flag_por(&j->p, cmd + 5);
}

static void desenhar_estacao(Jogo *j)
{
    static const struct { int x, y; const char *texto; } PLACAS[] = {
        { 2, 6, "BAR" }, { 16, 6, "MERCADO" }, { 31, 6, "ESTALEIRO" },
        { 46, 6, "ESCRITÓRIOS" }, { 22, 12, "HANGAR" }
    };
    const Piloto *p = &j->p;
    int i, tx, ty;

    tela_limpar();
    camera_centralizar(&j->cam, &j->estacao, j->ex, j->ey);
    mapa_desenhar(&j->estacao, &j->cam, 0);
    for (i = 0; i < (int)(sizeof PLACAS / sizeof PLACAS[0]); i++)
        if (camera_na_tela(&j->cam, PLACAS[i].x, PLACAS[i].y, &tx, &ty))
            tela_texto(tx, ty, CINZA_ESCURO, PRETO, PLACAS[i].texto);
    for (i = 0; i < n_pessoas; i++) {
        const Pessoa *k = &pessoas[i];
        if (k->x < 0 || !dialogo_condicao(k->condicao, condicao, j)) continue;
        if (camera_na_tela(&j->cam, k->x, k->y, &tx, &ty))
            tela_por(tx, ty, k->glifo, k->cor, PRETO);
    }
    if (camera_na_tela(&j->cam, j->ex, j->ey, &tx, &ty))
        tela_por(tx, ty, '@', BRANCO, PRETO);

    tela_retangulo(60, 0, 1, 18, G_V, CINZA_ESCURO, PRETO);
    tela_texto_max(62, 0, AMARELO, PRETO, SISTEMAS[p->sistema].nome, 17);
    tela_textof(62, 2, CINZA, PRETO, "Dia %d/%d", p->dia, PRAZO_DIAS);
    tela_textof(62, 4, AMARELO, PRETO, "Cr %ld", p->creditos);
    tela_textof(62, 5, VERMELHO_CLARO, PRETO, "Dívida %ld", p->divida);
    tela_textof(62, 7, CINZA, PRETO, "Porão %d/%d", porao_usado(p), porao_capacidade(p));
    tela_textof(62, 8, CINZA, PRETO, "Comb. %d/%d", p->combustivel, TANQUE);
    tela_textof(62, 9, CINZA, PRETO, "Casco %d/%d", p->casco, casco_maximo(p));
    tela_texto(62, 14, CINZA_ESCURO, PRETO, "c porão");
    tela_texto(62, 15, CINZA_ESCURO, PRETO, "Esc menu");
    tela_texto(62, 16, CINZA_ESCURO, PRETO, "nave = decolar");
    tela_retangulo(0, 18, TELA_L, 1, G_H, CINZA_ESCURO, PRETO);
    log_desenhar(&j->log, 1, 19, TELA_L - 2, 5);
}

static int pessoa_em(Jogo *j, int x, int y)
{
    int i;
    for (i = 0; i < n_pessoas; i++)
        if (pessoas[i].x == x && pessoas[i].y == y &&
            dialogo_condicao(pessoas[i].condicao, condicao, j))
            return i;
    return -1;
}

Modo estacao_rodar(Jogo *j)
{
    if (!carregar_estacao(j)) {
        ui_mensagem("Erro", "Não achei dados/estacao.txt.");
        j->fim = FIM_SAIU;
        return MODO_FIM;
    }
    log_adicionar(&j->log, CIANO_CLARO, "Atracada em %s.", SISTEMAS[j->p.sistema].nome);
    entregar_missoes(j);
    sortear_oferta(j);
    if (jogo_salvar(j, "estrela.sav"))
        log_adicionar(&j->log, CINZA_ESCURO, "(jogo salvo)");

    while (j->fim == EM_JOGO) {
        int tecla, dx = 0, dy = 0, alvo, tile;
        desenhar_estacao(j);
        tela_mostrar();
        tecla = term_esperar_tecla();
        switch (tecla) {
            case TECLA_CIMA:     dy = -1; break;
            case TECLA_BAIXO:    dy = 1;  break;
            case TECLA_ESQUERDA: dx = -1; break;
            case TECLA_DIREITA:  dx = 1;  break;
            case 'c': case 'C':  porao_tela(j); continue;
            case TECLA_ESC: {
                const char *op[] = { "Continuar", "Salvar e sair" };
                if (ui_menu("Menu", op, 2, 0) == 1) {
                    jogo_salvar(j, "estrela.sav");
                    j->fim = FIM_SAIU;
                }
                continue;
            }
            default: continue;
        }
        alvo = pessoa_em(j, j->ex + dx, j->ey + dy);
        tile = mapa_tile(&j->estacao, j->ex + dx, j->ey + dy);
        if (alvo < 0 && tile == E_BALCAO)
            alvo = pessoa_em(j, j->ex + 2 * dx, j->ey + 2 * dy);
        if (alvo >= 0) {
            dialogo_executar(&dialogos, pessoas[alvo].no, pessoas[alvo].nome,
                             condicao, efeito, j);
            continue;
        }
        if (mapa_bloqueia(&j->estacao, j->ex + dx, j->ey + dy)) continue;
        j->ex += dx;
        j->ey += dy;
        if (tile == E_NAVE && ui_confirmar("Embarcar na Estrela Errante e decolar?")) {
            voo_chegar(j, 1);
            log_adicionar(&j->log, CINZA, "Decolagem autorizada. Boa viagem, capitã.");
            return MODO_ESPACO;
        }
    }
    return MODO_FIM;
}
