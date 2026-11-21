/* teclas.c — mostra o código de cada tecla apertada. Q sai. */
#include <stdio.h>
#include "term.h"

int main(void)
{
    char linha[64];
    int tecla, n = 0;
    term_iniciar();
    term_escrever("\x1b[1;1HAperte teclas (Q sai):", 26);
    for (;;) {
        tecla = term_tecla();                 /* NÃO espera: devolve 0 se nada */
        if (tecla == TECLA_NENHUMA) {
            term_dormir(10);
            continue;
        }
        if (tecla == 'q' || tecla == 'Q') break;
        n = sprintf(linha, "\x1b[3;1H\x1b[2KCódigo %d %s   tempo %.2fs", tecla,
                    tecla == TECLA_CIMA ? "(seta para cima)" :
                    tecla == TECLA_BAIXO ? "(seta para baixo)" :
                    tecla == TECLA_ESQUERDA ? "(seta esquerda)" :
                    tecla == TECLA_DIREITA ? "(seta direita)" :
                    tecla == TECLA_ENTER ? "(Enter)" :
                    tecla == TECLA_ESC ? "(Esc)" : "", term_tempo());
        term_escrever(linha, (size_t)n);
    }
    term_encerrar();
    return 0;
}
