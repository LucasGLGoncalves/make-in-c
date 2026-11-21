/* encruzilhada.c — decisões com if, else e switch. */
#include <stdio.h>

int main(void)
{
    int escolha, tem_tocha = 0;
    char resposta;

    printf("Você está numa encruzilhada na floresta. Escurece.\n");
    printf("Há uma tocha apagada no chão. Pegar? (s/n) ");
    scanf(" %c", &resposta);
    if (resposta == 's' || resposta == 'S') {
        tem_tocha = 1;
        printf("Você guarda a tocha no cinto.\n");
    } else {
        printf("Você deixa a tocha onde está.\n");
    }

    printf("\nTrês caminhos:\n");
    printf("  1) A trilha da esquerda, que desce para um rio\n");
    printf("  2) A trilha do meio, que entra numa caverna\n");
    printf("  3) A trilha da direita, que sobe a montanha\n");
    printf("Escolha: ");
    scanf("%d", &escolha);

    switch (escolha) {
    case 1:
        printf("O rio está cheio. Você atravessa, molhado e com frio.\n");
        printf("FIM: você sobreviveu, mas pegou um resfriado.\n");
        break;
    case 2:
        if (tem_tocha) {
            printf("Você acende a tocha. Ouro brilha nas paredes!\n");
            printf("FIM: você está rico.\n");
        } else {
            printf("Está escuro demais. Você tropeça e volta.\n");
            printf("FIM: ainda pobre, mas inteiro.\n");
        }
        break;
    case 3:
        printf("Lá do alto você vê a aldeia inteira acesa.\n");
        printf("FIM: você encontrou o caminho de casa.\n");
        break;
    default:
        printf("Você fica parado até amanhecer. Os lobos agradecem.\n");
        break;
    }
    return 0;
}
