/* paleta.c — a tela virtual: cores, molduras, texto e a tabela CP437. */
#include "term.h"
#include "tela.h"

int main(void)
{
    int i, linha, coluna;
    term_iniciar();
    tela_limpar();

    tela_moldura(0, 0, 80, 24, 1, AZUL_CLARO, PRETO);
    tela_texto(3, 0, BRANCO, PRETO, " A tela virtual ");
    for (i = 0; i < 16; i++) {                     /* as 16 cores */
        tela_retangulo(3 + i * 4, 2, 4, 2, ' ', BRANCO, i);
        tela_textof(3 + i * 4, 4, i == 0 ? CINZA_ESCURO : i, PRETO, "%2d", i);
    }
    tela_texto(3, 6, AMARELO, PRETO, "Tabela CP437 (os 256 caracteres do PC):");
    for (linha = 0; linha < 16; linha++)
        for (coluna = 0; coluna < 16; coluna++)
            tela_por(4 + coluna * 2, 7 + linha, tela_cp437(linha * 16 + coluna),
                     (linha + coluna) % 2 ? CINZA : BRANCO, PRETO);

    tela_moldura(40, 7, 36, 8, 0, VERDE_CLARO, PRETO);
    tela_texto(42, 8, VERDE_CLARO, PRETO, "Acentos? Sem problema:");
    tela_texto(42, 9, BRANCO, PRETO, "ação, coração, avô, à toa");
    tela_por(42, 11, G_ROSTO_CHEIO, AMARELO, PRETO);
    tela_por(44, 11, G_OUROS, CIANO_CLARO, PRETO);
    tela_por(46, 11, G_COPAS, VERMELHO_CLARO, PRETO);
    tela_por(48, 11, G_SOL, AMARELO, PRETO);
    tela_por(50, 11, G_OMEGA, MAGENTA_CLARO, PRETO);
    tela_retangulo(52, 11, 4, 1, G_SOMBRA_LEVE, MARROM, PRETO);
    tela_retangulo(56, 11, 4, 1, G_SOMBRA_MEDIA, MARROM, PRETO);
    tela_retangulo(60, 11, 4, 1, G_SOMBRA_FORTE, MARROM, PRETO);
    tela_retangulo(64, 11, 4, 1, G_BLOCO, MARROM, PRETO);
    tela_texto(42, 13, CINZA_ESCURO, PRETO, "aperte qualquer tecla");
    tela_mostrar();
    term_esperar_tecla();
    term_encerrar();
    return 0;
}
