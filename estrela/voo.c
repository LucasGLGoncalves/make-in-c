/*
 * voo.c — o voo em tempo real: física, piratas, lasers, radar.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "rng.h"
#include "estrela.h"

#define VISTA_X   1          /* janela de voo dentro da moldura */
#define VISTA_Y   1
#define VISTA_L   58
#define VISTA_A   17
#define ASPECTO   0.5        /* uma linha mede ~2 colunas: y anda pela metade */
#define ESTACAO_X 34.0
#define ESTACAO_Y (-8.0)
#define PLANETA_X (-38.0)
#define PLANETA_Y 10.0
#define PI        3.14159265358979

/* As 8 direções, começando no norte e girando no sentido horário. */
static const double DIR_X[8] = { 0, 0.7071, 1, 0.7071, 0, -0.7071, -1, -0.7071 };
static const double DIR_Y[8] = { -1, -0.7071, 0, 0.7071, 1, 0.7071, 0, -0.7071 };
static const unsigned int GLIFO_NAVE[8] = {
    0x2191, 0x2197, 0x2192, 0x2198, 0x2193, 0x2199, 0x2190, 0x2196  /* ↑↗→↘↓↙←↖ */
};
static const char GLIFO_TIRO[8] = { '|', '/', '-', '\\', '|', '/', '-', '\\' };

/* Distância "visual": corrige o fato de as linhas serem mais altas. */
static double distancia(double x0, double y0, double x1, double y1)
{
    double dx = x1 - x0, dy = (y1 - y0) / ASPECTO;
    return sqrt(dx * dx + dy * dy);
}

static double velocidade(const Nave *n)
{
    return sqrt(n->vx * n->vx + (n->vy / ASPECTO) * (n->vy / ASPECTO));
}

/* Qual das 8 direções aponta para (dx, dy)? dy já em unidades visuais. */
static int direcao_para(double dx, double dy)
{
    double angulo = atan2(dx, -dy);              /* 0 = norte, sentido horário */
    int d = (int)floor(angulo / (PI / 4) + 0.5);
    return (d % 8 + 8) % 8;
}

static int virar_para(int atual, int desejada)
{
    int diferenca = (desejada - atual + 8) % 8;
    if (diferenca == 0) return atual;
    return diferenca <= 4 ? (atual + 1) % 8 : (atual + 7) % 8;
}

/* Um número "aleatório" fixo para cada célula: estrelas sem memória. */
static unsigned int ruido(int x, int y)
{
    unsigned int h = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static void atirar(Jogo *j, double x, double y, double vx, double vy, int dir, int do_jogador)
{
    int i;
    for (i = 0; i < MAX_TIROS; i++) {
        Tiro *t = &j->tiros[i];
        if (t->ativo) continue;
        t->ativo = 1;
        t->x = x + DIR_X[dir] * 1.2;
        t->y = y + DIR_Y[dir] * 1.2 * ASPECTO;
        t->vx = vx + DIR_X[dir] * 0.9;
        t->vy = vy + DIR_Y[dir] * 0.9 * ASPECTO;
        t->dir = dir;
        t->ttl = 35;
        t->do_jogador = do_jogador;
        return;
    }
}

static void explosao(Jogo *j, double x, double y, int pedacos)
{
    int i, criados = 0;
    for (i = 0; i < MAX_PARTICULAS && criados < pedacos; i++) {
        Particula *p = &j->part[i];
        double angulo = rng_intervalo(0, 359) * PI / 180.0;
        double forca = rng_intervalo(5, 40) / 100.0;
        if (p->ttl > 0) continue;
        p->x = x;
        p->y = y;
        p->vx = cos(angulo) * forca;
        p->vy = sin(angulo) * forca * ASPECTO;
        p->ttl = rng_intervalo(10, 30);
        criados++;
    }
}

static void criar_pirata(Jogo *j, double distancia_min)
{
    int i;
    for (i = 0; i < MAX_PIRATAS; i++) {
        Pirata *pi = &j->piratas[i];
        double angulo = rng_intervalo(0, 359) * PI / 180.0;
        double raio = distancia_min + rng_intervalo(0, 25);
        if (pi->ativo) continue;
        pi->ativo = 1;
        pi->x = j->nave.x + cos(angulo) * raio;
        pi->y = j->nave.y + sin(angulo) * raio * ASPECTO;
        pi->vx = pi->vy = 0;
        pi->dir = rng_intervalo(0, 7);
        pi->hp = 6 + SISTEMAS[j->p.sistema].perigo * 3;
        pi->recarga = 60;
        pi->giro = 0;
        return;
    }
}

void voo_chegar(Jogo *j, int perto_da_estacao)
{
    int i, n, perigo = SISTEMAS[j->p.sistema].perigo;
    Nave *nv = &j->nave;
    for (i = 0; i < MAX_TIROS; i++)      j->tiros[i].ativo = 0;
    for (i = 0; i < MAX_PARTICULAS; i++) j->part[i].ttl = 0;
    for (i = 0; i < MAX_CONTAINERS; i++) j->cont[i].ativo = 0;
    for (i = 0; i < MAX_PIRATAS; i++)    j->piratas[i].ativo = 0;

    nv->vx = nv->vy = 0;
    nv->acelerador = 0;
    nv->recarga = 0;
    nv->escudo = equip_valor(EQ_ESCUDO, j->p.equip[EQ_ESCUDO]);
    if (perto_da_estacao) {
        nv->x = ESTACAO_X; nv->y = ESTACAO_Y + 3; nv->dir = 4;
    } else {
        nv->x = -60; nv->y = 6; nv->dir = 2;    /* chega pela borda do sistema */
    }
    n = (perigo == 0) ? rng_chance(20) : perigo + rng_intervalo(0, 1);
    if (perto_da_estacao) n /= 2;
    for (i = 0; i < n; i++) criar_pirata(j, 45);
}

static void ferir_jogador(Jogo *j, int dano)
{
    Nave *nv = &j->nave;
    if (nv->escudo >= dano) {                 /* o escudo segura tudo */
        nv->escudo -= dano;
        return;
    }
    dano -= (int)nv->escudo;                  /* o que sobrar vai no casco */
    nv->escudo = 0;
    j->p.casco -= dano;
}

static void destruir_pirata(Jogo *j, Pirata *pi)
{
    int i;
    pi->ativo = 0;
    j->p.abates++;
    explosao(j, pi->x, pi->y, 25);
    if (SISTEMAS[j->p.sistema].economia != ECO_FORA_DA_LEI) {
        j->p.creditos += 250;
        log_adicionar(&j->log, VERDE_CLARO, "Pirata destruído! A patrulha paga 250 de recompensa.");
    } else {
        log_adicionar(&j->log, VERDE_CLARO, "Pirata destruído! (Aqui ninguém paga recompensa.)");
    }
    if (!rng_chance(60)) return;
    for (i = 0; i < MAX_CONTAINERS; i++)      /* deixa um contêiner para trás */
        if (!j->cont[i].ativo) {
            j->cont[i].ativo = 1;
            j->cont[i].x = pi->x;
            j->cont[i].y = pi->y;
            j->cont[i].bem = rng_intervalo(0, N_BENS - 1);
            j->cont[i].qtd = rng_intervalo(1, 4);
            return;
        }
}

static void pirata_pensar(Jogo *j, Pirata *pi)
{
    double dx = j->nave.x - pi->x, dy = (j->nave.y - pi->y) / ASPECTO;
    double dist = sqrt(dx * dx + dy * dy);
    int desejada = direcao_para(dx, dy);

    if (--pi->giro <= 0) {                     /* pirata gira devagar */
        pi->giro = 5;
        pi->dir = virar_para(pi->dir, desejada);
    }
    if (dist > 10) {
        pi->vx += DIR_X[pi->dir] * 0.025;
        pi->vy += DIR_Y[pi->dir] * 0.025 * ASPECTO;
    } else {
        pi->vx *= 0.94;
        pi->vy *= 0.94;
    }
    if (pi->recarga > 0) pi->recarga--;
    else if (pi->dir == desejada && dist < 28) {
        atirar(j, pi->x, pi->y, pi->vx, pi->vy, pi->dir, 0);
        pi->recarga = rng_intervalo(25, 45);
    }
    if (fabs(pi->vx) > 0.4) pi->vx *= 0.9;
    if (fabs(pi->vy) > 0.2) pi->vy *= 0.9;
    pi->x += pi->vx;
    pi->y += pi->vy;
}

static void atualizar(Jogo *j)
{
    Nave *nv = &j->nave;
    Piloto *p = &j->p;
    double empuxo = equip_valor(EQ_MOTOR, p->equip[EQ_MOTOR]) / 100.0;
    double escudo_max = equip_valor(EQ_ESCUDO, p->equip[EQ_ESCUDO]);
    int i, k;

    /* 1. a nave do jogador */
    if (nv->acelerador > 0) {
        nv->vx += DIR_X[nv->dir] * 0.008 * empuxo * nv->acelerador;
        nv->vy += DIR_Y[nv->dir] * 0.008 * empuxo * nv->acelerador * ASPECTO;
    } else if (nv->acelerador < 0) {          /* freio */
        nv->vx *= 0.92;
        nv->vy *= 0.92;
    }
    nv->vx *= 0.995;                          /* "atrito" para ficar jogável */
    nv->vy *= 0.995;
    if (velocidade(nv) > 0.6 * empuxo) {      /* velocidade máxima do motor */
        double f = 0.6 * empuxo / velocidade(nv);
        nv->vx *= f;
        nv->vy *= f;
    }
    nv->x += nv->vx;
    nv->y += nv->vy;
    if (nv->recarga > 0) nv->recarga--;
    if (nv->escudo < escudo_max) nv->escudo += 0.05;

    if (distancia(nv->x, nv->y, 0, 0) < 7 && j->tick % 10 == 0) {
        p->casco -= 1;
        if (j->tick % 60 == 0)
            log_adicionar(&j->log, VERMELHO_CLARO, "O calor da estrela derrete o casco!");
    }

    /* 2. piratas */
    for (i = 0; i < MAX_PIRATAS; i++)
        if (j->piratas[i].ativo) pirata_pensar(j, &j->piratas[i]);
    if (SISTEMAS[p->sistema].perigo >= 2 && j->tick % (TICKS_SEG * 20) == 0 &&
        rng_chance(40))
        criar_pirata(j, 60);

    /* 3. tiros: em dois meio-passos, para não atravessarem ninguém */
    for (i = 0; i < MAX_TIROS; i++) {
        Tiro *t = &j->tiros[i];
        int passo;
        if (!t->ativo) continue;
        for (passo = 0; passo < 2 && t->ativo; passo++) {
            t->x += t->vx / 2;
            t->y += t->vy / 2;
            if (t->do_jogador) {
                for (k = 0; k < MAX_PIRATAS; k++) {
                    Pirata *pi = &j->piratas[k];
                    if (!pi->ativo || distancia(t->x, t->y, pi->x, pi->y) > 1.2) continue;
                    t->ativo = 0;
                    pi->hp -= equip_valor(EQ_ARMA, p->equip[EQ_ARMA]);
                    explosao(j, t->x, t->y, 3);
                    if (pi->hp <= 0) destruir_pirata(j, pi);
                    break;
                }
            } else if (distancia(t->x, t->y, nv->x, nv->y) < 1.2) {
                t->ativo = 0;
                ferir_jogador(j, 4 + SISTEMAS[p->sistema].perigo);
                explosao(j, t->x, t->y, 4);
            }
        }
        if (--t->ttl <= 0) t->ativo = 0;
    }

    /* 4. partículas e contêineres */
    for (i = 0; i < MAX_PARTICULAS; i++)
        if (j->part[i].ttl > 0) {
            j->part[i].x += j->part[i].vx;
            j->part[i].y += j->part[i].vy;
            j->part[i].ttl--;
        }
    for (i = 0; i < MAX_CONTAINERS; i++) {
        Container *c = &j->cont[i];
        int livre = porao_capacidade(p) - porao_usado(p);
        if (!c->ativo || distancia(c->x, c->y, nv->x, nv->y) > 1.5) continue;
        if (livre <= 0) {
            if (j->tick % 30 == 0) log_adicionar(&j->log, CINZA, "Porão cheio: não cabe o contêiner.");
            continue;
        }
        if (c->qtd > livre) c->qtd = livre;
        p->carga[c->bem] += c->qtd;
        log_adicionar(&j->log, AMARELO, "Contêiner recolhido: %d t de %s.",
                      c->qtd, NOME_BEM[c->bem]);
        c->ativo = 0;
    }
}

static int na_vista(double cam_x, double cam_y, double x, double y, int *sx, int *sy)
{
    *sx = VISTA_X + (int)floor(x - cam_x);
    *sy = VISTA_Y + (int)floor(y - cam_y);
    return *sx >= VISTA_X && *sx < VISTA_X + VISTA_L &&
           *sy >= VISTA_Y && *sy < VISTA_Y + VISTA_A;
}

static void desenhar_ceu(const Jogo *j, double cam_x, double cam_y)
{
    int i, k;
    for (k = 0; k < VISTA_A; k++)
        for (i = 0; i < VISTA_L; i++) {
            int wx = (int)floor(cam_x) + i, wy = (int)floor(cam_y) + k;
            unsigned int h = ruido(wx, wy);
            double d_estrela = distancia(wx + 0.5, wy + 0.5, 0, 0);
            double d_planeta = distancia(wx + 0.5, wy + 0.5, PLANETA_X, PLANETA_Y);
            int sx = VISTA_X + i, sy = VISTA_Y + k;
            if (d_estrela < 5)
                tela_por(sx, sy, d_estrela < 3.5 ? G_BLOCO : G_SOMBRA_FORTE, AMARELO, VERMELHO);
            else if (d_estrela < 7)
                tela_por(sx, sy, G_SOMBRA_LEVE, VERMELHO_CLARO, PRETO);
            else if (d_planeta < 7)
                tela_por(sx, sy, (h % 3) ? G_SOMBRA_MEDIA : G_SOMBRA_FORTE,
                         (h % 5) ? CIANO : VERDE, AZUL);
            else if (h % 53 == 0)
                tela_por(sx, sy, G_PONTO, (h >> 8) % 3 ? CINZA_ESCURO : BRANCO, PRETO);
            else
                tela_por(sx, sy, ' ', CINZA, PRETO);
        }
    (void)j;
}

static void desenhar_estacao(double cam_x, double cam_y)
{
    /* a estação vista de fora:    ╔╩╗
     *                             ═╣■╠═
     *                              ╚╦╝      (0 = espaço vazio) */
    static const unsigned int FORMA[3][5] = {
        { 0,    G_SE2,    G_T_BAIXO2, G_SD2,    0    },
        { G_H2, G_T_DIR2, G_QUADRADO, G_T_ESQ2, G_H2 },
        { 0,    G_IE2,    G_T_CIMA2,  G_ID2,    0    }
    };
    int lin, col, sx, sy;
    for (lin = 0; lin < 3; lin++)
        for (col = 0; col < 5; col++) {
            unsigned int g = FORMA[lin][col];
            if (g == 0) continue;
            if (na_vista(cam_x, cam_y, ESTACAO_X - 2 + col, ESTACAO_Y - 1 + lin, &sx, &sy))
                tela_por(sx, sy, g, g == G_QUADRADO ? AMARELO : BRANCO, PRETO);
        }
}

static void desenhar_radar(const Jogo *j)
{
    int i, rx, ry;
    const int CX = 70, CY = 5;                 /* centro do radar na tela */
    ui_janela(60, 0, 20, 10, "Radar", VERDE);
    for (i = 0; i < 4; i++) {                  /* a trama verde do radar */
        tela_por(CX, CY - 4 + i * 2, G_PONTO, VERDE, PRETO);
        tela_por(CX - 8 + i * 4, CY, G_PONTO, VERDE, PRETO);
    }
#define PONTO_RADAR(wx, wy, glifo, cor) \
    rx = CX + (int)floor(((wx) - j->nave.x) / 8.0); \
    ry = CY + (int)floor(((wy) - j->nave.y) / 4.0); \
    if (rx > 60 && rx < 79 && ry > 0 && ry < 9) tela_por(rx, ry, glifo, cor, PRETO)

    PONTO_RADAR(0, 0, G_SOL, AMARELO);
    PONTO_RADAR(ESTACAO_X, ESTACAO_Y, G_QUADRADO, BRANCO);
    for (i = 0; i < MAX_CONTAINERS; i++)
        if (j->cont[i].ativo) { PONTO_RADAR(j->cont[i].x, j->cont[i].y, G_PONTO, AMARELO); }
    for (i = 0; i < MAX_PIRATAS; i++)
        if (j->piratas[i].ativo) { PONTO_RADAR(j->piratas[i].x, j->piratas[i].y, G_BOLINHA, VERMELHO_CLARO); }
    tela_por(CX, CY, '+', BRANCO, PRETO);
#undef PONTO_RADAR
}

static void desenhar_painel(const Jogo *j)
{
    const Piloto *p = &j->p;
    static const char *const ACEL[6] = { "FREIO", "parado", "1/4", "2/4", "3/4", "MÁXIMO" };
    int escudo_max = equip_valor(EQ_ESCUDO, p->equip[EQ_ESCUDO]);
    tela_texto(61, 10, CINZA, PRETO, "Escudo");
    ui_barra(68, 10, 11, (int)j->nave.escudo, escudo_max, AZUL_CLARO, CINZA_ESCURO);
    tela_texto(61, 11, CINZA, PRETO, "Casco");
    ui_barra(68, 11, 11, p->casco, casco_maximo(p),
             p->casco * 3 < casco_maximo(p) ? VERMELHO_CLARO : VERDE_CLARO, CINZA_ESCURO);
    tela_textof(61, 12, CINZA, PRETO, "Motor  %s", ACEL[j->nave.acelerador + 1]);
    tela_textof(61, 13, CINZA, PRETO, "Veloc. %.0f", velocidade(&j->nave) * TICKS_SEG);
    tela_textof(61, 14, CINZA, PRETO, "Comb.  %d/%d", p->combustivel, TANQUE);
    tela_textof(61, 15, CINZA, PRETO, "Porão  %d/%d", porao_usado(p), porao_capacidade(p));
    tela_textof(61, 16, AMARELO, PRETO, "Cr     %ld", p->creditos);
    tela_textof(61, 17, p->dia > PRAZO_DIAS - 10 ? VERMELHO_CLARO : CINZA, PRETO,
                "Dia    %d/%d", p->dia, PRAZO_DIAS);
}

static void desenhar_voo(const Jogo *j)
{
    double cam_x = j->nave.x - VISTA_L / 2.0, cam_y = j->nave.y - VISTA_A / 2.0;
    int i, sx, sy;

    tela_limpar();
    desenhar_ceu(j, cam_x, cam_y);
    desenhar_estacao(cam_x, cam_y);
    for (i = 0; i < MAX_CONTAINERS; i++)
        if (j->cont[i].ativo && na_vista(cam_x, cam_y, j->cont[i].x, j->cont[i].y, &sx, &sy))
            tela_por(sx, sy, G_QUADRADO, AMARELO, PRETO);
    for (i = 0; i < MAX_TIROS; i++) {
        const Tiro *t = &j->tiros[i];
        if (t->ativo && na_vista(cam_x, cam_y, t->x, t->y, &sx, &sy))
            tela_por(sx, sy, (unsigned char)GLIFO_TIRO[t->dir],
                     t->do_jogador ? VERDE_CLARO : VERMELHO_CLARO, PRETO);
    }
    for (i = 0; i < MAX_PIRATAS; i++) {
        const Pirata *pi = &j->piratas[i];
        if (pi->ativo && na_vista(cam_x, cam_y, pi->x, pi->y, &sx, &sy))
            tela_por(sx, sy, GLIFO_NAVE[pi->dir], VERMELHO_CLARO, PRETO);
    }
    for (i = 0; i < MAX_PARTICULAS; i++) {
        const Particula *p = &j->part[i];
        static const unsigned int FAISCA[3] = { G_PONTO, '+', '*' };
        if (p->ttl > 0 && na_vista(cam_x, cam_y, p->x, p->y, &sx, &sy))
            tela_por(sx, sy, FAISCA[p->ttl % 3], p->ttl > 15 ? AMARELO : VERMELHO, PRETO);
    }
    if (na_vista(cam_x, cam_y, j->nave.x, j->nave.y, &sx, &sy))
        tela_por(sx, sy, GLIFO_NAVE[j->nave.dir], BRANCO, PRETO);

    tela_moldura(0, 0, 60, 19, 1, AZUL, PRETO);
    tela_textof(2, 0, BRANCO, PRETO, " %s ", SISTEMAS[j->p.sistema].nome);
    tela_texto(2, 18, AZUL_CLARO, PRETO,
               " ←→ girar  ↑↓ motor  Espaço atira  D atraca  J salta ");
    desenhar_radar(j);
    desenhar_painel(j);
    log_desenhar(&j->log, 1, 19, TELA_L - 2, 5);
}

Modo voo_rodar(Jogo *j)
{
    const double PASSO = 1.0 / TICKS_SEG;
    double proximo = term_tempo();
    Nave *nv = &j->nave;

    for (;;) {
        int tecla;
        while ((tecla = term_tecla()) != TECLA_NENHUMA) {
            switch (tecla) {
            case TECLA_ESQUERDA:
            case TECLA_DIREITA:
                if (j->tick - nv->ultimo_giro >= 3) {   /* gira no máximo 10x/s */
                    nv->dir = (nv->dir + (tecla == TECLA_DIREITA ? 1 : 7)) % 8;
                    nv->ultimo_giro = (int)j->tick;
                }
                break;
            case TECLA_CIMA:  if (nv->acelerador < 4)  nv->acelerador++; break;
            case TECLA_BAIXO: if (nv->acelerador > -1) nv->acelerador--; break;
            case ' ':
                if (nv->recarga == 0) {
                    atirar(j, nv->x, nv->y, nv->vx, nv->vy, nv->dir, 1);
                    nv->recarga = 8;
                }
                break;
            case 'd': case 'D':
                if (distancia(nv->x, nv->y, ESTACAO_X, ESTACAO_Y) > 4)
                    log_adicionar(&j->log, CINZA, "Longe demais da estação para atracar.");
                else if (velocidade(nv) > 0.35)
                    log_adicionar(&j->log, CINZA, "Rápido demais! Reduza para atracar.");
                else
                    return MODO_ESTACAO;
                break;
            case 'j': case 'J':
                if (distancia(nv->x, nv->y, 0, 0) < 30) {
                    log_adicionar(&j->log, CINZA, "A gravidade da estrela impede o salto. Afaste-se.");
                } else {
                    int destino = mapa_estelar(j);
                    if (destino >= 0) saltar(j, destino);
                    proximo = term_tempo();
                }
                break;
            case 'p': case 'P':
                ui_mensagem("Pausa", "O universo espera por você.");
                proximo = term_tempo();
                break;
            case TECLA_ESC:
                if (ui_confirmar("Sair do jogo sem salvar? (Para salvar, atraque.)")) {
                    j->fim = FIM_SAIU;
                    return MODO_FIM;
                }
                proximo = term_tempo();
                break;
            }
        }

        if (term_tempo() - proximo > 0.25) proximo = term_tempo();
        while (term_tempo() >= proximo) {
            atualizar(j);
            j->tick++;
            proximo += PASSO;
        }

        if (j->p.casco <= 0) {
            int q;
            for (q = 0; q < 40; q++) {                     /* a última explosão */
                if (q % 4 == 0) explosao(j, nv->x, nv->y, 12);
                atualizar(j);
                desenhar_voo(j);
                tela_mostrar();
                term_dormir(30);
            }
            j->fim = FIM_DESTRUIDA;
            return MODO_FIM;
        }
        if (j->p.dia > PRAZO_DIAS && j->p.divida > 0) {
            j->fim = FIM_PRAZO;
            return MODO_FIM;
        }

        desenhar_voo(j);
        tela_mostrar();
        term_esperar_ate(proximo);
    }
}
