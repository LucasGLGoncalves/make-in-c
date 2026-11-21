/* caverna.c — o laço principal de uma aventura de texto. */
#include <stdio.h>

int main(void)
{
    int sala = 0;          /* 0 entrada, 1 salão, 2 lago, 3 saída */
    int jogando = 1;
    char comando;

    while (jogando) {
        printf("\n");
        if (sala == 0) printf("ENTRADA. A luz do dia fica para trás. Caminho ao norte.\n");
        if (sala == 1) printf("SALÃO. Estalactites pingam. Passagens a leste e ao sul.\n");
        if (sala == 2) printf("LAGO. Água negra e parada. Uma brisa vem do norte.\n");
        if (sala == 3) {
            printf("Você sente o vento no rosto e vê as estrelas. Saiu da caverna!\n");
            break;                                     /* sai do while */
        }
        printf("Direção (n/s/l/o) ou q para sair: ");
        scanf(" %c", &comando);

        switch (comando) {
        case 'n':
            if (sala == 0) sala = 1;
            else if (sala == 2) sala = 3;
            else printf("Uma parede de pedra.\n");
            break;
        case 's':
            if (sala == 1) sala = 0;
            else printf("Não há passagem ao sul.\n");
            break;
        case 'l':
            if (sala == 1) sala = 2;
            else printf("Não há passagem a leste.\n");
            break;
        case 'o':
            if (sala == 2) sala = 1;
            else printf("Não há passagem a oeste.\n");
            break;
        case 'q':
            jogando = 0;
            break;
        default:
            printf("Não entendi.\n");
        }
    }
    printf("Fim de jogo.\n");
    return 0;
}
