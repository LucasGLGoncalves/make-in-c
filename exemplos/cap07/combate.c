/* combate.c — um combate por turnos dividido em funções. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rolar(int lados)
{
    return rand() % lados + 1;
}

/* Calcula o dano de um golpe; 0 se errou. */
int golpe(int ataque, int defesa)
{
    int dado = rolar(20);
    if (dado == 1) return 0;                  /* erro crítico   */
    if (dado == 20) return ataque * 2;        /* acerto crítico */
    if (dado + ataque <= 10 + defesa) return 0;
    return rolar(ataque);
}

void mostrar_vida(const char *nome, int vida, int maximo)
{
    int i;
    printf("%-8s [", nome);
    for (i = 0; i < maximo; i++)
        putchar(i < vida ? '#' : '.');
    printf("] %d/%d\n", vida, maximo);
}

int main(void)
{
    int vida_heroi = 20, vida_orc = 16, rodada = 1;
    char acao;
    srand((unsigned int)time(NULL));

    printf("Um orc bloqueia a ponte!\n");
    while (vida_heroi > 0 && vida_orc > 0) {
        int dano;
        printf("\n--- Rodada %d ---\n", rodada++);
        mostrar_vida("Você", vida_heroi, 20);
        mostrar_vida("Orc", vida_orc, 16);
        printf("(a)tacar ou (d)efender? ");
        scanf(" %c", &acao);

        if (acao == 'a') {
            dano = golpe(6, 2);
            if (dano > 0) printf("Você acerta o orc: %d de dano!\n", dano);
            else          printf("Você erra o golpe.\n");
            vida_orc -= dano;
        } else {
            printf("Você ergue o escudo.\n");
        }
        if (vida_orc <= 0) break;

        dano = golpe(5, acao == 'd' ? 6 : 2);  /* defender aumenta a defesa */
        if (dano > 0) printf("O orc acerta você: %d de dano!\n", dano);
        else          printf("O orc erra.\n");
        vida_heroi -= dano;
    }
    printf(vida_heroi > 0 ? "\nO orc cai da ponte. Vitória!\n"
                          : "\nVocê cai... Fim de jogo.\n");
    return 0;
}
