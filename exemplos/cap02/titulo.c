/* titulo.c — uma tela de título feita só com printf. */
#include <stdio.h>

int main(void)
{
    /* Cada \n pula uma linha. A barra invertida "\\" precisa ser dobrada,
     * e as aspas dentro do texto viram \". */
    printf("\n");
    printf("   +--------------------------------------+\n");
    printf("   |                                      |\n");
    printf("   |     A  C A V E R N A  D O  E C O     |\n");
    printf("   |                                      |\n");
    printf("   |          /\\      /\\      /\\          |\n");
    printf("   |         /  \\    /  \\    /  \\         |\n");
    printf("   |   _____/    \\__/    \\__/    \\_____   |\n");
    printf("   |                                      |\n");
    printf("   +--------------------------------------+\n");
    printf("\n");
    printf("   \"Quem entra na caverna ouve a própria voz\n");
    printf("    responder perguntas que ainda não fez.\"\n\n");
    printf("   [Aperte Enter para começar]\n");
    getchar();      /* espera o jogador apertar Enter */
    return 0;
}
