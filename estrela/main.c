/*
 * main.c — Estrela Errante.
 * Projeto 4 do livro "Make in C: jogos de aventuras".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "estrela.h"

#define ARQUIVO_SAVE "estrela.sav"

static void desenhar_titulo(void)
{
    static const char *LOGO[5] = {
        "#### ### ##### ####  #### #     ###",
        "#    #     #   #   # #    #    #   #",
        "###   ##   #   ####  ###  #    #####",
        "#       #  #   #  #  #    #    #   #",
        "#### ###   #   #   # #### #### #   #"
    };
    int i, x, y;
    tela_limpar();
    for (i = 0; i < 120; i++)                         /* um céu fixo */
        tela_por((i * 53) % TELA_L, (i * 29) % TELA_A, G_PONTO,
                 i % 4 ? CINZA_ESCURO : BRANCO, PRETO);
    for (y = 0; y < 5; y++)
        for (x = 0; LOGO[y][x] != '\0'; x++)
            if (LOGO[y][x] == '#')
                tela_por(22 + x, 1 + y, G_BLOCO, y < 2 ? CIANO_CLARO : AZUL_CLARO, PRETO);
    tela_texto_centro(7, AMARELO, PRETO, "E  R  R  A  N  T  E");
    tela_por(57, 7, 0x2197u, BRANCO, PRETO);
    tela_texto_centro(22, CINZA_ESCURO, PRETO,
                      "Projeto 4 de \"Make in C: jogos de aventuras\"");
}

static void ajuda(void)
{
    ui_mensagem("Como jogar",
        "NO ESPAÇO: ← e → giram a nave, ↑ e ↓ mudam a marcha do motor "
        "(a última é o freio). Espaço atira. Perto da estação e devagar, D "
        "atraca. Longe da estrela, J abre o mapa para saltar.\n\n"
        "NA ESTAÇÃO: ande com as setas e esbarre nas pessoas para conversar. "
        "O terminal verde é o mercado; a engenheira cuida da nave; o agente "
        "oferece entregas. Pise na sua nave para decolar.\n\n"
        "OBJETIVO: pagar os 20.000 créditos ao Banco Orbital, em Nova Lisboa, "
        "até o dia 100.");
}

static void novo_jogo(Jogo *j, const char *nome)
{
    Piloto *p = &j->p;
    memset(p, 0, sizeof *p);
    snprintf(p->nome, sizeof p->nome, "%s", nome);
    p->sistema = 0;                       /* Nova Lisboa, o 1º do arquivo */
    p->dia = 1;
    p->creditos = 1500;
    p->divida = DIVIDA_INICIAL;
    p->casco = casco_maximo(p);
    p->combustivel = TANQUE;
    universo_sortear_precos(p->sistema);
    ui_mensagem("Herança",
        "Seu tio-avô deixou para você uma nave cargueira de quarenta anos, a "
        "Estrela Errante... e uma dívida de 20.000 créditos com o Banco "
        "Orbital de Nova Lisboa.\n\nO banco deu um prazo: dia 100. Se a dívida "
        "não estiver paga, a nave vai a leilão.\n\nVocê tem 1.500 créditos, um "
        "porão vazio e um setor inteiro pela frente.");
}

int main(void)
{
    static Jogo jogo;
    int colunas, linhas;

    term_tamanho(&colunas, &linhas);
    if (colunas < TELA_L || linhas < TELA_A) {
        printf("Aumente o terminal para %dx%d.\n", TELA_L, TELA_A);
        return 1;
    }
    if (!universo_carregar() || !estacao_iniciar()) {
        printf("Não achei a pasta dados/. Rode o jogo de dentro da pasta estrela.\n");
        return 1;
    }
    term_iniciar();
    rng_semear(rng_semente_do_relogio());

    for (;;) {
        const char *opcoes[4];
        int acoes[4], n = 0, e;
        Modo modo = MODO_ESTACAO;
        FILE *teste = fopen(ARQUIVO_SAVE, "r");
        if (teste) { fclose(teste); opcoes[n] = "Continuar"; acoes[n++] = 1; }
        opcoes[n] = "Novo jogo";  acoes[n++] = 0;
        opcoes[n] = "Como jogar"; acoes[n++] = 2;
        opcoes[n] = "Sair";       acoes[n++] = 3;

        desenhar_titulo();
        tela_mostrar();
        e = ui_menu("Menu", opcoes, n, 0);
        if (e < 0 || acoes[e] == 3) break;
        if (acoes[e] == 2) { ajuda(); continue; }

        log_limpar(&jogo.log);
        jogo.fim = EM_JOGO;
        if (acoes[e] == 1) {
            if (!jogo_carregar(&jogo, ARQUIVO_SAVE)) {
                ui_mensagem("Erro", "O jogo salvo está estragado.");
                continue;
            }
            universo_sortear_precos(jogo.p.sistema);
        } else {
            char nome[24];
            if (!ui_ler_texto("Nome da capitã:", nome, sizeof nome)) continue;
            novo_jogo(&jogo, nome);
        }

        /* a máquina de estados: espaço <-> estação, até o fim */
        while (modo != MODO_FIM) {
            if (modo == MODO_ESTACAO) modo = estacao_rodar(&jogo);
            else                      modo = voo_rodar(&jogo);
        }

        if (jogo.fim == FIM_VITORIA) {
            char texto[256];
            remove(ARQUIVO_SAVE);
            snprintf(texto, sizeof texto, "Dívida quitada no dia %d, com %ld créditos "
                     "sobrando e %d piratas a menos no setor.\n\nA Estrela Errante "
                     "agora é sua. Para onde vai?",
                     jogo.p.dia, jogo.p.creditos, jogo.p.abates);
            ui_mensagem("Vitória!", texto);
        } else if (jogo.fim == FIM_DESTRUIDA) {
            remove(ARQUIVO_SAVE);
            ui_mensagem("Fim", "A Estrela Errante se desfaz em mil pedaços brilhantes. "
                        "O Banco Orbital vai ter que procurar outra devedora.");
        } else if (jogo.fim == FIM_PRAZO) {
            remove(ARQUIVO_SAVE);
            ui_mensagem("Fim", "Dia 100. Os oficiais do Banco Orbital lacram a escotilha "
                        "da Estrela Errante. A nave vai a leilão, e você, para o chão.");
        }
    }

    estacao_encerrar();
    mapa_destruir(&jogo.estacao);
    term_encerrar();
    return 0;
}
