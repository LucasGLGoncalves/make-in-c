/* ficha.c — variáveis, tipos e leitura do teclado. */
#include <stdio.h>

int main(void)
{
    char   inicial;          /* um único caractere          */
    int    idade;            /* um número inteiro           */
    int    forca = 10;       /* já começa valendo 10        */
    double ouro = 12.5;      /* número com casas decimais   */
    const int VIDA_MAXIMA = 30;

    printf("=== Criação de personagem ===\n");
    printf("Qual a inicial do seu nome? ");
    scanf(" %c", &inicial);
    printf("Quantos anos tem o seu herói? ");
    scanf("%d", &idade);

    printf("\n--- Ficha ---\n");
    printf("Inicial: %c\n", inicial);
    printf("Idade:   %d anos\n", idade);
    printf("Força:   %d\n", forca);
    printf("Vida:    %d/%d\n", VIDA_MAXIMA, VIDA_MAXIMA);
    printf("Ouro:    %.2f moedas\n", ouro);

    forca = forca + idade / 10;        /* a experiência fortalece */
    printf("Depois de treinar, a força virou %d.\n", forca);
    printf("O tipo int ocupa %d bytes; o double, %d.\n",
           (int)sizeof(int), (int)sizeof(double));
    return 0;
}
