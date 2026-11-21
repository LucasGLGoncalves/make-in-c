/*
 * main.c — A Coroa de Ferro.
 * Projeto 2 do livro "Make in C: jogos de aventuras".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "fov.h"
#include "coroa.h"

#define ARQUIVO_SAVE "coroa.sav"

static void desenhar_titulo(void)
{
    /* uma coroa feita de blocos */
    static const char *COROA[6] = {
        "  #     #     #  ",
        "  ##   ###   ##  ",
        "  ### ##### ###  ",
        "  #############  ",
        "  #o###o###o###  ",
        "  #############  "
    };
    int y;
    tela_limpar();
    for (y = 0; y < 6; y++) {
        int x;
        for (x = 0; COROA[y][x] != '\0'; x++) {
            char c = COROA[y][x];
            if (c == '#') tela_por(31 + x, y, G_BLOCO, CINZA, PRETO);
            if (c == 'o') tela_por(31 + x, y, G_OUROS, VERMELHO_CLARO, CINZA);
        }
    }
    tela_texto_centro(6, BRANCO, PRETO, "A   C O R O A   D E   F E R R O");
    tela_texto_centro(7, CINZA_ESCURO, PRETO, "uma aventura no Vale Cinzento");
    tela_texto_centro(22, CINZA_ESCURO, PRETO,
                      "Projeto 2 de \"Make in C: jogos de aventuras\"");
}

static void ajuda(void)
{
    ui_mensagem("Como jogar",
        "Setas ou WASD: andar. Ande contra um monstro para atacar e contra "
        "um morador para conversar (vale por cima de balcões).\n\n"
        "Espaço: esperar um turno.   i: mochila e equipamento.\n"
        "> e <: descer e subir escadas.   S: salvar e sair.\n\n"
        "Itens são recolhidos ao pisar neles. Você recupera 1 de vida a cada "
        "10 turnos. Encontre o Rei Corvo no quinto nível da abadia.");
}

static void tela_lapide(const Jogo *j)
{
    const Heroi *h = &j->heroi;
    tela_limpar();
    tela_retangulo(28, 3, 24, 14, ' ', CINZA, CINZA_ESCURO);
    tela_retangulo(31, 2, 18, 1, ' ', CINZA, CINZA_ESCURO);
    tela_retangulo(20, 17, 40, 1, G_SOMBRA_LEVE, VERDE, PRETO);
    tela_texto_centro(5, PRETO, CINZA_ESCURO, "DESCANSE");
    tela_texto_centro(6, PRETO, CINZA_ESCURO, "EM PAZ");
    tela_texto_centro(8, BRANCO, CINZA_ESCURO, h->nome);
    tela_texto_centro(10, PRETO, CINZA_ESCURO, "derrubado por");
    tela_texto_centro(11, PRETO, CINZA_ESCURO, j->causa_morte);
    {
        char linha[40];
        sprintf(linha, "no nível %d", j->profundidade);
        tela_texto_centro(12, PRETO, CINZA_ESCURO, linha);
        sprintf(linha, "%d monstros, %d de ouro", h->abatidos, h->ouro);
        tela_texto_centro(14, PRETO, CINZA_ESCURO, linha);
    }
    tela_texto_centro(20, CINZA_ESCURO, PRETO, "[tecla]");
    tela_mostrar();
    term_esperar_tecla();
}

static void partida(Jogo *j)
{
    int tecla;
    while (j->fim == JOGANDO) {
        if (j->profundidade > 0)
            fov_calcular(&j->mapa, j->heroi.x, j->heroi.y, 8);
        desenhar_jogo(j);
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == '?') { ajuda(); continue; }
        if (tecla == 'S') {
            if (ui_confirmar("Salvar o jogo e sair?")) {
                if (jogo_salvar(j, ARQUIVO_SAVE)) j->fim = FIM_SALVOU;
                else ui_mensagem("Erro", "Não consegui gravar o arquivo.");
            }
            continue;
        }
        if (!heroi_acao(j, tecla)) continue;      /* não gastou turno */

        /* o mundo reage à ação do herói */
        if (j->profundidade > 0)
            fov_calcular(&j->mapa, j->heroi.x, j->heroi.y, 8);
        monstros_turno(j);
        j->heroi.turnos++;
        if (j->heroi.turnos % 10 == 0 && j->heroi.hp < j->heroi.hp_max)
            j->heroi.hp++;
        if (j->heroi.hp <= 0) j->fim = FIM_MORTE;
    }
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
    if (!dados_carregar_tudo() || !aldeia_iniciar()) {
        printf("Não achei os arquivos da pasta dados/. Rode o jogo de dentro "
               "da pasta do projeto.\n");
        return 1;
    }
    term_iniciar();
    rng_semear(rng_semente_do_relogio());

    for (;;) {
        FILE *teste = fopen(ARQUIVO_SAVE, "rb");
        int tem_save = (teste != NULL), e;
        const char *opcoes[4];
        int n = 0, acoes[4];
        if (teste) fclose(teste);
        if (tem_save) { opcoes[n] = "Continuar"; acoes[n++] = 1; }
        opcoes[n] = "Nova aventura"; acoes[n++] = 0;
        opcoes[n] = "Como jogar";    acoes[n++] = 2;
        opcoes[n] = "Sair";          acoes[n++] = 3;

        desenhar_titulo();
        tela_mostrar();
        e = ui_menu("Menu", opcoes, n, 0);
        if (e < 0 || acoes[e] == 3) break;
        if (acoes[e] == 2) { ajuda(); continue; }

        log_limpar(&jogo.log);
        jogo.fim = JOGANDO;
        if (acoes[e] == 1) {
            if (!jogo_carregar(&jogo, ARQUIVO_SAVE)) {
                ui_mensagem("Erro", "O arquivo de save está estragado.");
                continue;
            }
            log_adicionar(&jogo.log, CINZA, "Bem-vindo de volta, %s.", jogo.heroi.nome);
        } else {
            char nome[24];
            if (!ui_ler_texto("Qual é o seu nome, aventureiro?", nome, sizeof nome))
                continue;
            heroi_criar(&jogo.heroi, nome);
            jogo.n_flags = 0;
            mundo_ir_para(&jogo, 0, 1);
            log_adicionar(&jogo.log, AMARELO,
                          "%s chega a Vale Cinzento. O Ancião Tobias quer falar com você.",
                          nome);
        }

        partida(&jogo);

        if (jogo.fim == FIM_MORTE) {
            remove(ARQUIVO_SAVE);            /* morte é para sempre */
            tela_lapide(&jogo);
        } else if (jogo.fim == FIM_VITORIA) {
            char texto[256];
            remove(ARQUIVO_SAVE);
            sprintf(texto, "%s devolveu a Coroa de Ferro ao Vale Cinzento em "
                    "%ld turnos, derrotando %d criaturas.\n\nOs bardos vão "
                    "cantar esta história por muito tempo.",
                    jogo.heroi.nome, jogo.heroi.turnos, jogo.heroi.abatidos);
            ui_mensagem("Vitória!", texto);
        }
    }

    aldeia_encerrar();
    mapa_destruir(&jogo.mapa);
    free(jogo.dist);
    term_encerrar();
    return 0;
}
