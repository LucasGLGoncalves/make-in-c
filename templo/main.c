/*
 * main.c — O Templo da Serpente.
 * Projeto 1 do livro "Make in C: jogos de aventuras".
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "templo.h"

static const char *LETRAS[5] = {
    "##### ##### #   # ####  #      ### ",
    "  #   #     ## ## #   # #     #   #",
    "  #   ####  # # # ####  #     #   #",
    "  #   #     #   # #     #     #   #",
    "  #   ##### #   # #     #####  ### "
};

static void desenhar_titulo(void)
{
    static const int CORES[5] = { AMARELO, AMARELO, MARROM, MARROM, VERMELHO };
    int linha, i;
    tela_limpar();
    for (linha = 0; linha < 5; linha++)
        for (i = 0; LETRAS[linha][i] != '\0'; i++)
            if (LETRAS[linha][i] == '#') {        /* cada '#' vira 2 blocos */
                tela_por(5 + i * 2,     1 + linha, G_BLOCO, CORES[linha], PRETO);
                tela_por(5 + i * 2 + 1, 1 + linha, G_BLOCO, CORES[linha], PRETO);
            }
    tela_texto_centro(7, VERDE_CLARO, PRETO, "~ da  Serpente ~");

    /* legenda: o menu vai aparecer no meio, entre a linha 8 e a 15 */
    tela_por(14, 16, G_ROSTO_CHEIO, AMARELO, PRETO);
    tela_texto(16, 16, CINZA, PRETO, "você");
    tela_por(14, 17, G_OUROS, CIANO_CLARO, PRETO);
    tela_texto(16, 17, CINZA, PRETO, "gema (é a sua vida)");
    tela_por(14, 18, 0x2310u, BRANCO, PRETO);
    tela_texto(16, 18, CINZA, PRETO, "chicote");
    tela_por(14, 19, G_FEMININO, AMARELO, PRETO);
    tela_texto(16, 19, CINZA, PRETO, "chave");
    tela_por(44, 16, G_A_TREMA, VERMELHO_CLARO, PRETO);
    tela_por(45, 16, G_O_TREMA, VERDE_CLARO, PRETO);
    tela_por(46, 16, G_OMEGA, MAGENTA_CLARO, PRETO);
    tela_texto(48, 16, CINZA, PRETO, "monstros");
    tela_por(44, 17, G_SOL, MAGENTA_CLARO, PRETO);
    tela_texto(48, 17, CINZA, PRETO, "orbe de teleporte");
    tela_por(44, 18, G_TRIPLO, PRETO, MAGENTA_CLARO);
    tela_texto(48, 18, CINZA, PRETO, "escada (saída)");
    tela_por(44, 19, G_FI, AMARELO, VERMELHO);
    tela_texto(48, 19, CINZA, PRETO, "o Ídolo");
    tela_texto_centro(21, CINZA, PRETO,
        "Desça os quatro níveis do templo e roube o Ídolo da Serpente.");
    tela_texto_centro(23, CINZA_ESCURO, PRETO,
        "Setas: andar   C: chicote   T: teleporte   P: pausa   Esc: sair");
}

static void jogar(Jogo *j)
{
    const double PASSO = 1.0 / TICKS_SEG;
    double proximo = term_tempo();
    int tecla;

    j->situacao = RODANDO;
    while (j->situacao == RODANDO) {
        /* 1. ENTRADA: trata todas as teclas que chegaram */
        while ((tecla = term_tecla()) != TECLA_NENHUMA) {
            switch (tecla) {
                case TECLA_CIMA:     jogador_mover(j, 0, -1); break;
                case TECLA_BAIXO:    jogador_mover(j, 0, 1);  break;
                case TECLA_ESQUERDA: jogador_mover(j, -1, 0); break;
                case TECLA_DIREITA:  jogador_mover(j, 1, 0);  break;
                case 'c': case 'C':  jogador_chicotear(j);    break;
                case 't': case 'T':  jogador_teleportar(j);   break;
                case 'p': case 'P':
                    ui_mensagem("Pausa", "O tempo parou no templo.");
                    proximo = term_tempo();
                    break;
                case TECLA_ESC:
                    if (ui_confirmar("Abandonar a aventura?"))
                        j->situacao = DESISTIU;
                    proximo = term_tempo();
                    break;
            }
        }

        /* 2. LÓGICA: um passo a cada 1/TICKS_SEG segundos */
        if (term_tempo() - proximo > 0.25)        /* atrasou demais? */
            proximo = term_tempo();               /* então esquece   */
        while (term_tempo() >= proximo && j->situacao == RODANDO) {
            monstros_atualizar(j);
            if (j->aviso_ticks > 0) j->aviso_ticks--;
            j->tick++;
            proximo += PASSO;
        }
        if (j->jog.gemas < 0) j->situacao = MORREU;

        /* 3. DESENHO */
        desenhar_jogo(j);
        tela_mostrar();

        /* 4. ESPERA até a hora do próximo passo */
        term_esperar_ate(proximo);
    }
}

static void tela_final(const Jogo *j)
{
    char texto[256];
    if (j->situacao == VENCEU)
        sprintf(texto, "O Ídolo da Serpente é seu! A luz do sol entra pela "
                "rachadura do teto enquanto você sobe com o tesouro.\n\n"
                "Pontuação final: %ld", j->jog.pontos);
    else if (j->situacao == MORREU)
        sprintf(texto, "Suas gemas se apagaram no nível %d. O templo guarda "
                "mais um aventureiro para sempre.\n\nPontuação: %ld",
                j->nivel, j->jog.pontos);
    else
        sprintf(texto, "Você deu meia-volta e saiu do templo.\n\n"
                "Pontuação: %ld", j->jog.pontos);
    ui_mensagem(j->situacao == VENCEU ? "Vitória!" : "Fim de jogo", texto);
}

int main(void)
{
    static Jogo jogo;              /* static: começa zerada e fica fora da pilha */
    const char *opcoes[] = { "Nova aventura", "Recordes", "Sair" };
    int colunas, linhas, escolha;

    term_tamanho(&colunas, &linhas);
    if (colunas < TELA_L || linhas < TELA_A) {
        printf("Aumente o terminal para %dx%d (agora: %dx%d).\n",
               TELA_L, TELA_A, colunas, linhas);
        return 1;
    }
    term_iniciar();
    rng_semear(rng_semente_do_relogio());

    for (;;) {
        desenhar_titulo();
        tela_mostrar();
        escolha = ui_menu("O Templo da Serpente", opcoes, 3, 0);
        if (escolha == 0) {
            jogador_iniciar(&jogo.jog);
            if (!nivel_carregar(&jogo, 1)) {
                ui_mensagem("Erro", "Não achei niveis/nivel1.txt. Rode o jogo "
                            "de dentro da pasta do projeto.");
                continue;
            }
            jogar(&jogo);
            tela_final(&jogo);
            if (jogo.situacao != DESISTIU)
                recordes_registrar(jogo.jog.pontos);
        } else if (escolha == 1) {
            recordes_mostrar();
        } else {
            break;                     /* "Sair" ou Esc */
        }
    }
    mapa_destruir(&jogo.mapa);
    term_encerrar();
    return 0;
}
