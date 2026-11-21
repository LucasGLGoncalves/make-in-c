/*
 * main.c — Porto Sombrio.
 * Projeto 3 do livro "Make in C: jogos de aventuras".
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "noir.h"

static void tela_titulo(Caso *c)
{
    int tecla = TECLA_NENHUMA;
    /* a tela de título já tem chuva: usamos o mapa do escritório como cenário */
    while (tecla == TECLA_NENHUMA) {
        int i;
        tela_limpar();
        for (i = 0; i < MAX_GOTAS; i++)
            tela_por(c->gotas[i].x % TELA_L, c->gotas[i].y % TELA_A, '/', AZUL, PRETO);
        tela_texto_centro(8,  BRANCO,  PRETO, "P  O  R  T  O");
        tela_texto_centro(10, MAGENTA_CLARO, PRETO, "S  O  M  B  R  I  O");
        tela_texto_centro(13, CINZA,   PRETO, "Uma investigação em uma noite de chuva");
        tela_texto_centro(20, CINZA_ESCURO, PRETO, "[aperte uma tecla]");
        tela_mostrar();
        chuva_atualizar(c);
        term_dormir(60);
        tecla = term_tecla();
    }
}

static void agir(Caso *c, int tecla)
{
    int dx = 0, dy = 0, nx, ny, alvo, tile;
    switch (tecla) {
        case TECLA_CIMA:     dy = -1; break;
        case TECLA_BAIXO:    dy = 1;  break;
        case TECLA_ESQUERDA: dx = -1; break;
        case TECLA_DIREITA:  dx = 1;  break;
        case 'n': case 'N':  caso_caderno(c); return;
        case '?':
            ui_mensagem("Como jogar",
                "Setas: andar. Ande contra uma pessoa ou objeto para conversar "
                "ou examinar (vale por cima de balcões). Pise no ponto de táxi "
                "(seta amarela) para ir a outro lugar.\n\nN: caderno de pistas.  "
                "Esc: desistir.\n\nO delegado arquiva o caso às 6h da manhã. "
                "Conversas e corridas de táxi gastam tempo.");
            return;
        case TECLA_ESC:
            if (ui_confirmar("Abandonar o caso?")) c->situacao = DESISTIU;
            return;
        default:
            return;
    }
    nx = c->x + dx;
    ny = c->y + dy;
    alvo = coisa_em(c, nx, ny);
    tile = mapa_tile(&c->mapa, nx, ny);
    if (alvo < 0 && tile == NT_BALCAO)          /* fala por cima do balcão */
        alvo = coisa_em(c, nx + dx, ny + dy);
    if (alvo >= 0) {
        caso_interagir(c, alvo);
        return;
    }
    if (mapa_bloqueia(&c->mapa, nx, ny)) return;
    c->x = nx;
    c->y = ny;
    if (tile == NT_SAIDA) caso_taxi(c);
}

static void investigar(Caso *c)
{
    while (c->situacao == INVESTIGANDO) {
        int tecla;
        /* enquanto a detetive pensa, a chuva cai e o neon pisca */
        while ((tecla = term_tecla()) == TECLA_NENHUMA) {
            chuva_atualizar(c);
            desenhar_caso(c);
            tela_mostrar();
            term_dormir(50);
        }
        agir(c, tecla);
        if (c->situacao == INVESTIGANDO && c->minutos >= PRAZO_MINUTOS)
            c->situacao = FINAL_PRAZO;
    }
}

int main(void)
{
    static Caso caso;
    const char *titulo = "Fim";
    const char *texto = "";
    int colunas, linhas;
    char resumo[200];

    term_tamanho(&colunas, &linhas);
    if (colunas < TELA_L || linhas < TELA_A) {
        printf("Aumente o terminal para %dx%d.\n", TELA_L, TELA_A);
        return 1;
    }
    if (!caso_iniciar() || !local_carregar(&caso, "escritorio", 0)) {
        printf("Não achei a pasta dados/. Rode o jogo de dentro da pasta noir.\n");
        return 1;
    }
    term_iniciar();
    rng_semear(rng_semente_do_relogio());
    caso.situacao = INVESTIGANDO;
    log_limpar(&caso.log);

    tela_titulo(&caso);
    ui_mensagem("Sexta-feira, 20h00",
        "Chove há três dias em Porto Sombrio. A água escorre pela janela do "
        "seu escritório no terceiro andar da Rua do Carmo, e o letreiro do "
        "bar da esquina pisca em cor-de-rosa sobre a calçada.\n\n"
        "Você é Ana Torres, detetive particular. O aluguel está atrasado, o "
        "telefone foi cortado e a garrafa está quase vazia.\n\n"
        "Então alguém bate à porta.");
    log_adicionar(&caso.log, CINZA,
                  "Uma mulher espera por você no escritório. (? mostra a ajuda)");

    investigar(&caso);

    switch (caso.situacao) {
        case FINAL_BOM:    titulo = "Caso encerrado";
                           texto = "Justiça para Celeste Vidal."; break;
        case FINAL_FRACO:  titulo = "Caso arquivado";
                           texto = "Faltaram provas. A cidade esquece rápido."; break;
        case FINAL_ERRADO: titulo = "Caso errado";
                           texto = "Um inocente pagou. O culpado ri por aí."; break;
        case FINAL_PRAZO:  titulo = "Seis da manhã";
                           texto = "O delegado Ramos assinou o arquivamento. "
                                   "Para a polícia, Celeste se matou."; break;
        default:           titulo = "Fim";
                           texto = "Você largou o caso. A chuva continua."; break;
    }
    snprintf(resumo, sizeof resumo, "%s\n\nPistas encontradas: %d de %d.",
             texto, caso.n_pistas, caso_total_pistas());
    ui_mensagem(titulo, resumo);

    caso_encerrar();
    mapa_destruir(&caso.mapa);
    term_encerrar();
    return 0;
}
