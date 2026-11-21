/*
 * jogador.c — movimento, itens, chicote e teletransporte.
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "rng.h"
#include "templo.h"

void jogador_iniciar(Jogador *p)
{
    p->gemas    = 20;
    p->chicotes = 5;
    p->chaves   = 0;
    p->orbes    = 1;
    p->pontos   = 0;
}

void avisar(Jogo *j, const char *texto)
{
    snprintf(j->aviso, sizeof j->aviso, "%s", texto);
    j->aviso_ticks = TICKS_SEG * 3;          /* três segundos na tela */
}

/* O jogador acabou de pisar em (x,y): recolhe o que houver ali. */
static void pegar_item(Jogo *j, int x, int y)
{
    Jogador *p = &j->jog;
    switch (mapa_tile(&j->mapa, x, y)) {
        case TILE_GEMA:
            p->gemas++;   p->pontos += 10;
            break;
        case TILE_CHICOTE:
            p->chicotes++; p->pontos += 10;
            avisar(j, "Um chicote! Aperte C para usá-lo.");
            break;
        case TILE_CHAVE:
            p->chaves++;  p->pontos += 20;
            avisar(j, "Uma chave antiga. Ela abre uma porta.");
            break;
        case TILE_ORBE:
            p->orbes++;   p->pontos += 30;
            avisar(j, "Um orbe de teleporte! Aperte T para usá-lo.");
            break;
        case TILE_TESOURO:
            p->pontos += 250;
            avisar(j, "Tesouro! +250 pontos.");
            break;
        case TILE_SAIDA:
            p->pontos += 1000;
            if (j->nivel < TOTAL_NIVEIS) {
                nivel_carregar(j, j->nivel + 1);
                avisar(j, "Você desce mais fundo no templo...");
            }
            return;                       /* o mapa mudou: não mexer nele */
        case TILE_IDOLO:
            p->pontos += 5000;
            j->situacao = VENCEU;
            return;
        default:
            return;                       /* chão vazio: nada a fazer */
    }
    mapa_por(&j->mapa, x, y, TILE_CHAO);  /* o item sai do mapa */
}

void jogador_mover(Jogo *j, int dx, int dy)
{
    Jogador *p = &j->jog;
    int nx = p->x + dx, ny = p->y + dy;
    int tile = mapa_tile(&j->mapa, nx, ny);
    int alvo = monstro_em(j, nx, ny);

    if (alvo >= 0) {                      /* esbarrou num monstro */
        monstro_atingir_jogador(j, &j->monstros[alvo]);
        return;
    }
    if (tile == TILE_PORTA) {
        if (p->chaves > 0) {
            p->chaves--;
            mapa_por(&j->mapa, nx, ny, TILE_CHAO);
            avisar(j, "A chave gira e a porta se abre.");
        } else {
            avisar(j, "A porta está trancada. Você precisa de uma chave.");
        }
        return;
    }
    if (mapa_bloqueia(&j->mapa, nx, ny)) return;   /* parede, água, mato */

    p->x = nx;
    p->y = ny;
    if (mapa_flags(&j->mapa, nx, ny) & T_LAVA) {
        p->gemas -= 2;
        avisar(j, "A lava queima! -2 gemas.");
    }
    pegar_item(j, nx, ny);
}

void jogador_chicotear(Jogo *j)
{
    /* as 8 casas em volta, em sentido horário, e o desenho de cada uma */
    static const int  DX[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
    static const int  DY[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    static const char RISCO[8] = { '\\', '|', '/', '-', '\\', '|', '/', '-' };
    Jogador *p = &j->jog;
    int i;

    if (p->chicotes <= 0) {
        avisar(j, "Você não tem chicotes.");
        return;
    }
    p->chicotes--;

    for (i = 0; i < 8; i++) {
        int x = p->x + DX[i], y = p->y + DY[i];
        int alvo = monstro_em(j, x, y);
        int tile = mapa_tile(&j->mapa, x, y);

        if (alvo >= 0) {
            j->monstros[alvo].vivo = 0;
            p->pontos += 20 * (j->monstros[alvo].tipo + 1);
        }
        if (tile == TILE_QUEBRAVEL || (tile == TILE_MATO && rng_chance(50)))
            mapa_por(&j->mapa, x, y, TILE_CHAO);

        /* animação: desenha o jogo e o risco do chicote por cima */
        desenhar_jogo(j);
        tela_por(x, y, (unsigned char)RISCO[i], BRANCO, PRETO);
        tela_mostrar();
        term_dormir(25);
    }
}

void jogador_teleportar(Jogo *j)
{
    Jogador *p = &j->jog;
    int tentativa;
    if (p->orbes <= 0) {
        avisar(j, "Você não tem orbes de teleporte.");
        return;
    }
    p->orbes--;
    for (tentativa = 0; tentativa < 1000; tentativa++) {
        int x = rng_intervalo(1, j->mapa.largura - 2);
        int y = rng_intervalo(1, j->mapa.altura - 2);
        if (mapa_tile(&j->mapa, x, y) == TILE_CHAO && monstro_em(j, x, y) < 0) {
            p->x = x;
            p->y = y;
            avisar(j, "O mundo gira... você está em outro lugar!");
            return;
        }
    }
}
