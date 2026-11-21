/*
 * salvar.c — grava e lê o jogo num arquivo binário.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fov.h"
#include "coroa.h"

#define ASSINATURA "COROA"
#define VERSAO_SAVE 1

typedef struct {
    char assinatura[6];      /* "COROA" + '\0': é mesmo um save nosso? */
    int  versao;             /* muda quando o formato mudar             */
} Cabecalho;

int jogo_salvar(const Jogo *j, const char *arquivo)
{
    Cabecalho c;
    size_t n = (size_t)(j->mapa.largura * j->mapa.altura);
    FILE *f = fopen(arquivo, "wb");
    if (f == NULL) return 0;

    memset(&c, 0, sizeof c);
    strcpy(c.assinatura, ASSINATURA);
    c.versao = VERSAO_SAVE;
    fwrite(&c, sizeof c, 1, f);

    fwrite(&j->heroi, sizeof j->heroi, 1, f);
    fwrite(&j->profundidade, sizeof j->profundidade, 1, f);
    fwrite(&j->n_flags, sizeof j->n_flags, 1, f);
    fwrite(j->flags, sizeof j->flags, 1, f);

    /* o mapa tem ponteiros: gravamos o tamanho e depois o conteúdo */
    fwrite(&j->mapa.largura, sizeof(int), 1, f);
    fwrite(&j->mapa.altura, sizeof(int), 1, f);
    fwrite(j->mapa.tiles, 1, n, f);
    fwrite(j->mapa.visto, 1, n, f);

    fwrite(&j->n_atores, sizeof j->n_atores, 1, f);
    fwrite(j->atores, sizeof j->atores[0], (size_t)j->n_atores, f);
    fwrite(&j->n_chao, sizeof j->n_chao, 1, f);
    fwrite(j->chao, sizeof j->chao[0], (size_t)j->n_chao, f);

    return fclose(f) == 0;
}

#define LER(ptr, tam, qtd) if (fread((ptr), (tam), (qtd), f) != (size_t)(qtd)) goto erro

int jogo_carregar(Jogo *j, const char *arquivo)
{
    Cabecalho c;
    int largura, altura;
    size_t n;
    FILE *f = fopen(arquivo, "rb");
    if (f == NULL) return 0;

    LER(&c, sizeof c, 1);
    if (strcmp(c.assinatura, ASSINATURA) != 0 || c.versao != VERSAO_SAVE)
        goto erro;                           /* arquivo estranho ou antigo */

    LER(&j->heroi, sizeof j->heroi, 1);
    LER(&j->profundidade, sizeof j->profundidade, 1);
    LER(&j->n_flags, sizeof j->n_flags, 1);
    LER(j->flags, sizeof j->flags, 1);
    LER(&largura, sizeof(int), 1);
    LER(&altura, sizeof(int), 1);
    if (largura <= 0 || altura <= 0 || largura > 1000 || altura > 1000) goto erro;

    mapa_destruir(&j->mapa);
    if (!mapa_criar(&j->mapa, largura, altura, TILES)) goto erro;
    n = (size_t)(largura * altura);
    LER(j->mapa.tiles, 1, n);
    LER(j->mapa.visto, 1, n);

    LER(&j->n_atores, sizeof j->n_atores, 1);
    if (j->n_atores < 0 || j->n_atores > MAX_ATORES) goto erro;
    LER(j->atores, sizeof j->atores[0], j->n_atores);
    LER(&j->n_chao, sizeof j->n_chao, 1);
    if (j->n_chao < 0 || j->n_chao > MAX_CHAO) goto erro;
    LER(j->chao, sizeof j->chao[0], j->n_chao);
    fclose(f);

    free(j->dist);
    j->dist = malloc(sizeof(int) * n);
    j->cam.tela_x = 0;  j->cam.tela_y = 0;
    j->cam.largura = VISTA_L;  j->cam.altura = VISTA_A;
    if (j->profundidade == 0) fov_revelar_tudo(&j->mapa);
    return j->dist != NULL;

erro:
    fclose(f);
    return 0;
}
