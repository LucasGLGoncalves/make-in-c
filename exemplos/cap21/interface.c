/* interface.c — um passeio pelas peças de interface do motor. */
#include <stdio.h>
#include "term.h"
#include "tela.h"
#include "ui.h"

int main(void)
{
    Log log;
    char nome[24] = "Viajante";
    int vida = 20, tecla = 0;
    const char *poderes[] = { "Bola de fogo", "Cura", "Invisibilidade", "Voar" };

    term_iniciar();
    log_limpar(&log);
    log_adicionar(&log, AMARELO, "Bem-vindo! Aperte as teclas listadas ao lado.");

    while (tecla != 'q') {
        tela_limpar();
        ui_janela(0, 0, 50, 16, "Registro", CIANO);
        log_desenhar(&log, 2, 1, 46, 14);
        ui_janela(52, 0, 28, 16, nome, AMARELO);
        tela_textof(54, 2, CINZA, PRETO, "Vida %d/20", vida);
        ui_barra(54, 3, 20, vida, 20, vida < 7 ? VERMELHO_CLARO : VERDE_CLARO, CINZA_ESCURO);
        tela_texto(54, 5, CINZA, PRETO, "+ / -  vida");
        tela_texto(54, 6, CINZA, PRETO, "m  menu de poderes");
        tela_texto(54, 7, CINZA, PRETO, "j  janela de texto");
        tela_texto(54, 8, CINZA, PRETO, "n  trocar o nome");
        tela_texto(54, 9, CINZA, PRETO, "q  sair");
        ui_texto_quebrado(2, 18, 76, 5, CINZA_ESCURO, PRETO,
            "Esta linha é longa de propósito, para mostrar a quebra automática de "
            "texto: as palavras nunca são cortadas ao meio, e uma palavra que não "
            "cabe no fim da linha desce inteira para a linha de baixo.");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == '+' && vida < 20) { vida++; log_adicionar(&log, VERDE_CLARO, "Você se sente melhor."); }
        if (tecla == '-' && vida > 0)  { vida--; log_adicionar(&log, VERMELHO_CLARO, "Ai! Uma farpa."); }
        if (tecla == 'm') {
            int e = ui_menu("Qual poder?", poderes, 4, 0);
            if (e >= 0) log_adicionar(&log, MAGENTA_CLARO, "Você usou: %s.", poderes[e]);
            else        log_adicionar(&log, CINZA, "Menu cancelado.");
        }
        if (tecla == 'j')
            ui_mensagem("Uma janela", "As janelas guardam a tela que estava embaixo "
                        "e a devolvem quando fecham.\n\nAperte uma tecla.");
        if (tecla == 'n' && ui_ler_texto("Novo nome:", nome, sizeof nome))
            log_adicionar(&log, AMARELO, "Agora você se chama %s.", nome);
        if (tecla == 'q' && !ui_confirmar("Sair mesmo?")) tecla = 0;
    }
    term_encerrar();
    return 0;
}
