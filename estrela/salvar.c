/*
 * salvar.c — o jogo salvo em TEXTO, uma informação por linha:
 * "chave=valor". Dá para abrir no bloco de notas e entender.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dados.h"
#include "estrela.h"

int jogo_salvar(const Jogo *j, const char *arquivo)
{
    const Piloto *p = &j->p;
    int i;
    FILE *f = fopen(arquivo, "w");
    if (f == NULL) return 0;

    fprintf(f, "# Estrela Errante - jogo salvo\n");
    fprintf(f, "versao=1\n");
    fprintf(f, "nome=%s\n", p->nome);
    fprintf(f, "sistema=%s\n", SISTEMAS[p->sistema].id);   /* pelo id, não pelo número */
    fprintf(f, "dia=%d\n", p->dia);
    fprintf(f, "creditos=%ld\n", p->creditos);
    fprintf(f, "divida=%ld\n", p->divida);
    fprintf(f, "casco=%d\n", p->casco);
    fprintf(f, "combustivel=%d\n", p->combustivel);
    fprintf(f, "abates=%d\n", p->abates);
    fprintf(f, "equip=%d,%d,%d,%d,%d\n", p->equip[0], p->equip[1],
            p->equip[2], p->equip[3], p->equip[4]);
    fprintf(f, "carga=");
    for (i = 0; i < N_BENS; i++)
        fprintf(f, "%d%s", p->carga[i], i < N_BENS - 1 ? "," : "\n");
    for (i = 0; i < p->n_missoes; i++) {
        const Missao *m = &p->missoes[i];
        fprintf(f, "missao=%s,%d,%d,%d,%d\n", SISTEMAS[m->destino].id,
                m->bem, m->qtd, m->pagamento, m->prazo);
    }
    for (i = 0; i < p->n_flags; i++)
        fprintf(f, "flag=%s\n", p->flags[i]);
    return fclose(f) == 0;
}

static int sistema_por_id(const char *id)
{
    int i;
    for (i = 0; i < N_SISTEMAS; i++)
        if (strcmp(SISTEMAS[i].id, id) == 0) return i;
    return -1;
}

int jogo_carregar(Jogo *j, const char *arquivo)
{
    Piloto *p = &j->p;
    char linha[256];
    int versao = 0;
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) return 0;

    memset(p, 0, sizeof *p);
    p->sistema = -1;
    while (dados_ler_linha(f, linha, sizeof linha)) {
        char *igual = strchr(linha, '=');
        char *chave, *valor;
        if (igual == NULL) continue;
        *igual = '\0';                         /* corta a linha em duas */
        chave = dados_aparar(linha);
        valor = dados_aparar(igual + 1);

        if      (strcmp(chave, "versao") == 0)      versao = atoi(valor);
        else if (strcmp(chave, "nome") == 0)        snprintf(p->nome, sizeof p->nome, "%s", valor);
        else if (strcmp(chave, "sistema") == 0)     p->sistema = sistema_por_id(valor);
        else if (strcmp(chave, "dia") == 0)         p->dia = atoi(valor);
        else if (strcmp(chave, "creditos") == 0)    p->creditos = atol(valor);
        else if (strcmp(chave, "divida") == 0)      p->divida = atol(valor);
        else if (strcmp(chave, "casco") == 0)       p->casco = atoi(valor);
        else if (strcmp(chave, "combustivel") == 0) p->combustivel = atoi(valor);
        else if (strcmp(chave, "abates") == 0)      p->abates = atoi(valor);
        else if (strcmp(chave, "equip") == 0)
            sscanf(valor, "%d,%d,%d,%d,%d", &p->equip[0], &p->equip[1],
                   &p->equip[2], &p->equip[3], &p->equip[4]);
        else if (strcmp(chave, "carga") == 0) {
            char *campos[N_BENS];
            int n = dados_separar(valor, ',', campos, N_BENS), i;
            for (i = 0; i < n; i++) p->carga[i] = atoi(campos[i]);
        } else if (strcmp(chave, "missao") == 0 && p->n_missoes < MAX_MISSOES) {
            Missao *m = &p->missoes[p->n_missoes];
            char destino[32];
            if (sscanf(valor, "%31[^,],%d,%d,%d,%d", destino, &m->bem, &m->qtd,
                       &m->pagamento, &m->prazo) == 5 &&
                (m->destino = sistema_por_id(destino)) >= 0)
                p->n_missoes++;
        } else if (strcmp(chave, "flag") == 0) {
            flag_por(p, valor);
        }
    }
    fclose(f);
    return versao == 1 && p->sistema >= 0 && p->dia > 0;
}
