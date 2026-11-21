/* binario.c — gravando e lendo uma struct inteira com fwrite e fread. */
#include <stdio.h>

typedef struct {
    char nome[16];
    int  nivel, vida, ouro;
} Ficha;

int main(void)
{
    Ficha heroi = { "Lia", 3, 27, 140 }, lida;
    FILE *f = fopen("ficha.bin", "wb");          /* "b" = binário */
    if (f == NULL) return 1;
    fwrite(&heroi, sizeof heroi, 1, f);
    fclose(f);

    f = fopen("ficha.bin", "rb");
    if (f == NULL) return 1;
    if (fread(&lida, sizeof lida, 1, f) != 1) {
        printf("Arquivo curto demais!\n");
        fclose(f);
        return 1;
    }
    fclose(f);
    printf("%s, nível %d, %d de vida, %d de ouro (%d bytes no disco)\n",
           lida.nome, lida.nivel, lida.vida, lida.ouro, (int)sizeof lida);
    return 0;
}
