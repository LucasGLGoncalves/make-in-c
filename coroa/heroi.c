/*
 * heroi.c — o herói: atributos, mochila, equipamento e ações.
 */
#include <stdio.h>
#include <string.h>
#include "term.h"
#include "tela.h"
#include "ui.h"
#include "coroa.h"

void heroi_criar(Heroi *h, const char *nome)
{
    memset(h, 0, sizeof *h);
    snprintf(h->nome, sizeof h->nome, "%s", nome);
    h->hp = h->hp_max = 20;
    h->ataque_base = 2;
    h->defesa_base = 0;
    h->nivel = 1;
    h->ouro = 25;
    mochila_adicionar(h, item_por_id("adaga"), 1);
    mochila_adicionar(h, item_por_id("gibao"), 1);
    mochila_adicionar(h, item_por_id("pocao_cura"), 2);
    h->mochila[0].equipado = 1;             /* começa com a adaga na mão */
    h->mochila[1].equipado = 1;             /* e o gibão no corpo        */
}

int heroi_ataque(const Heroi *h)
{
    int total = h->ataque_base, i;
    for (i = 0; i < h->n_mochila; i++)
        if (h->mochila[i].equipado) total += ITENS[h->mochila[i].def].ataque;
    return total;
}

int heroi_defesa(const Heroi *h)
{
    int total = h->defesa_base, i;
    for (i = 0; i < h->n_mochila; i++)
        if (h->mochila[i].equipado) total += ITENS[h->mochila[i].def].defesa;
    return total;
}

void heroi_ganhar_xp(Jogo *j, int xp)
{
    Heroi *h = &j->heroi;
    h->xp += xp;
    while (h->xp >= h->nivel * 20) {         /* subiu de nível! */
        h->xp -= h->nivel * 20;
        h->nivel++;
        h->hp_max += 6;
        h->hp += 6;
        h->ataque_base++;
        if (h->nivel % 2 == 0) h->defesa_base++;
        log_adicionar(&j->log, AMARELO,
                      "Você se sente mais forte! Agora está no nível %d.", h->nivel);
    }
}

static int empilhavel(int def)
{
    return ITENS[def].tipo == IT_POCAO || ITENS[def].tipo == IT_PERGAMINHO;
}

int mochila_adicionar(Heroi *h, int def, int qtd)
{
    int i;
    if (def < 0) return 0;
    if (empilhavel(def))                     /* junta com a pilha que já existe */
        for (i = 0; i < h->n_mochila; i++)
            if (h->mochila[i].def == def) {
                h->mochila[i].qtd += qtd;
                return 1;
            }
    if (h->n_mochila >= MAX_MOCHILA) return 0;
    h->mochila[h->n_mochila].def = def;
    h->mochila[h->n_mochila].qtd = qtd;
    h->mochila[h->n_mochila].equipado = 0;
    h->n_mochila++;
    return 1;
}

void mochila_remover(Heroi *h, int indice, int qtd)
{
    h->mochila[indice].qtd -= qtd;
    if (h->mochila[indice].qtd > 0) return;
    h->n_mochila--;                          /* fecha o buraco na lista */
    memmove(&h->mochila[indice], &h->mochila[indice + 1],
            (size_t)(h->n_mochila - indice) * sizeof h->mochila[0]);
}

int mochila_procurar(const Heroi *h, const char *id)
{
    int i;
    for (i = 0; i < h->n_mochila; i++)
        if (strcmp(ITENS[h->mochila[i].def].id, id) == 0) return i;
    return -1;
}

int slot_ocupado(const Heroi *h, int slot)
{
    int i;
    for (i = 0; i < h->n_mochila; i++)
        if (h->mochila[i].equipado && ITENS[h->mochila[i].def].slot == slot)
            return i;
    return -1;
}

/* Usa ou equipa o item 'i' da mochila. Devolve 1 se gastou um turno. */
static int usar_item(Jogo *j, int i)
{
    Heroi *h = &j->heroi;
    ItemPilha *p = &h->mochila[i];
    const ItemDef *d = &ITENS[p->def];

    switch (d->tipo) {
    case IT_POCAO:
        h->hp += d->efeito;
        if (h->hp > h->hp_max) h->hp = h->hp_max;
        log_adicionar(&j->log, VERMELHO_CLARO,
                      "Você bebe a %s e recupera %d de vida.", d->nome, d->efeito);
        mochila_remover(h, i, 1);
        return 1;
    case IT_PERGAMINHO:
        memset(j->mapa.visto, 1, (size_t)(j->mapa.largura * j->mapa.altura));
        log_adicionar(&j->log, AMARELO,
                      "As runas brilham: o nível inteiro surge em sua mente.");
        mochila_remover(h, i, 1);
        return 1;
    case IT_ARMA:
    case IT_ARMADURA:
        if (p->equipado) {
            p->equipado = 0;
            log_adicionar(&j->log, CINZA, "Você guarda: %s.", d->nome);
        } else {
            int antigo = slot_ocupado(h, d->slot);
            if (antigo >= 0) h->mochila[antigo].equipado = 0;
            p->equipado = 1;
            log_adicionar(&j->log, CINZA, "Você equipa: %s.", d->nome);
        }
        return 1;
    default:
        log_adicionar(&j->log, CINZA, "Não há o que fazer com isso.");
        return 0;
    }
}

static void largar_item(Jogo *j, int i)
{
    Heroi *h = &j->heroi;
    ItemPilha *p = &h->mochila[i];
    if (ITENS[p->def].tipo == IT_COROA) {
        log_adicionar(&j->log, CINZA, "Depois de tudo? Jamais.");
        return;
    }
    chao_por(j, p->def, p->qtd, h->x, h->y);
    log_adicionar(&j->log, CINZA, "Você larga: %s.", ITENS[p->def].nome);
    p->qtd = 0;
    p->equipado = 0;
    mochila_remover(h, i, 0);
}

/* O "boneco" de equipamento, inspirado nas telas de RPG dos anos 90. */
static void desenhar_slot(int x, int y, const char *rotulo, const Heroi *h, int slot)
{
    int i = slot_ocupado(h, slot);
    const char *nome = (i >= 0) ? ITENS[h->mochila[i].def].nome : "(vazio)";
    tela_moldura(x, y, 25, 3, 0, i >= 0 ? BRANCO : CINZA_ESCURO, PRETO);
    tela_texto(x + 2, y, CINZA, PRETO, rotulo);
    tela_texto_max(x + 2, y + 1, i >= 0 ? BRANCO : CINZA_ESCURO, PRETO, nome, 21);
}

static void desenhar_boneco(const Heroi *h)
{
    int y;
    /* o corpo, desenhado com blocos azuis */
    tela_retangulo(14, 4, 4, 2, G_BLOCO, AZUL, PRETO);    /* cabeça */
    tela_retangulo(11, 6, 10, 6, G_BLOCO, AZUL, PRETO);   /* tronco */
    tela_retangulo(8, 7, 3, 5, G_BLOCO, AZUL, PRETO);     /* braço esq. */
    tela_retangulo(21, 7, 3, 5, G_BLOCO, AZUL, PRETO);    /* braço dir. */
    tela_retangulo(12, 12, 3, 6, G_BLOCO, AZUL, PRETO);   /* perna esq. */
    tela_retangulo(17, 12, 3, 6, G_BLOCO, AZUL, PRETO);   /* perna dir. */
    /* as caixas de cada parte do corpo, ligadas por linhas verdes */
    desenhar_slot(3, 0, "Cabeça", h, SLOT_CABECA);
    desenhar_slot(0, 18, "Mão (arma)", h, SLOT_MAO);
    desenhar_slot(0, 21, "Escudo", h, SLOT_ESCUDO);
    desenhar_slot(26, 18, "Corpo", h, SLOT_CORPO);
    desenhar_slot(26, 21, "Anel", h, SLOT_ANEL);
    tela_por(15, 2, G_T_CIMA, BRANCO, PRETO);
    for (y = 3; y < 4; y++) tela_por(15, y, G_V, VERDE_CLARO, PRETO);
    tela_por(7, 9, '-', VERDE_CLARO, PRETO);
    tela_por(24, 9, '-', VERDE_CLARO, PRETO);
    tela_textof(31, 1, VERDE_CLARO, PRETO, "Ataque %d   Defesa %d",
                heroi_ataque(h), heroi_defesa(h));
}

int inventario_tela(Jogo *j)
{
    Heroi *h = &j->heroi;
    for (;;) {
        int i, tecla, escolha;
        tela_limpar();
        desenhar_boneco(h);
        ui_janela(52, 0, 28, TELA_A, "Mochila", CIANO);
        for (i = 0; i < h->n_mochila; i++) {
            const ItemDef *d = &ITENS[h->mochila[i].def];
            int y = 2 + i;
            tela_textof(54, y, AMARELO, PRETO, "%c", 'a' + i);
            tela_por(56, y, d->glifo, d->cor, PRETO);
            tela_texto_max(58, y, CINZA, PRETO, d->nome, 15);
            if (h->mochila[i].qtd > 1)
                tela_textof(74, y, CINZA, PRETO, "x%d", h->mochila[i].qtd);
            if (h->mochila[i].equipado) tela_por(77, y, '*', VERDE_CLARO, PRETO);
        }
        if (h->n_mochila == 0) tela_texto(54, 2, CINZA_ESCURO, PRETO, "(vazia)");
        tela_texto(54, 21, CINZA_ESCURO, PRETO, "letra: escolher");
        tela_texto(54, 22, CINZA_ESCURO, PRETO, "Esc: voltar");
        tela_mostrar();

        tecla = term_esperar_tecla();
        if (tecla == TECLA_ESC || tecla == 'i') return 0;
        i = tecla - 'a';
        if (i < 0 || i >= h->n_mochila) continue;

        {
            const ItemDef *d = &ITENS[h->mochila[i].def];
            const char *acoes[3];
            char titulo[64];
            acoes[0] = (d->tipo == IT_ARMA || d->tipo == IT_ARMADURA)
                       ? (h->mochila[i].equipado ? "Desequipar" : "Equipar")
                       : "Usar";
            acoes[1] = "Largar no chão";
            acoes[2] = "Examinar";
            snprintf(titulo, sizeof titulo, "%s", d->nome);
            escolha = ui_menu(titulo, acoes, 3, 0);
            if (escolha == 0) return usar_item(j, i);
            if (escolha == 1) { largar_item(j, i); return 1; }
            if (escolha == 2) {
                char texto[256];
                snprintf(texto, sizeof texto, "%s\n\nAtaque +%d   Defesa +%d   "
                         "Valor %d", d->descricao, d->ataque, d->defesa, d->valor);
                ui_mensagem(d->nome, texto);
            }
        }
    }
}

static void pegar_itens(Jogo *j)
{
    Heroi *h = &j->heroi;
    int i;
    while ((i = chao_em(j, h->x, h->y)) >= 0) {
        ItemChao c = j->chao[i];
        const ItemDef *d = &ITENS[c.def];
        if (d->tipo == IT_OURO) {
            h->ouro += c.qtd;
            log_adicionar(&j->log, AMARELO, "Você recolhe %d moedas de ouro.", c.qtd);
        } else if (mochila_adicionar(h, c.def, c.qtd)) {
            log_adicionar(&j->log, BRANCO, "Você pega: %s.", d->nome);
        } else {
            log_adicionar(&j->log, CINZA, "Sua mochila está cheia.");
            return;
        }
        j->chao[i] = j->chao[--j->n_chao];   /* tira o item do chão */

        if (d->tipo == IT_COROA) {
            ui_mensagem("A Coroa de Ferro",
                "Seus dedos tocam o ferro frio e a coroa canta. Um vento "
                "antigo levanta você pelas escadas, pelos corredores, pela "
                "terra... e você abre os olhos na entrada da abadia, sob o sol.\n\n"
                "Leve a coroa ao Ancião Tobias.");
            mundo_ir_para(j, 0, 0);
            log_adicionar(&j->log, AMARELO, "Leve a coroa ao Ancião Tobias.");
            return;
        }
    }
}

int heroi_acao(Jogo *j, int tecla)
{
    Heroi *h = &j->heroi;
    int dx = 0, dy = 0, nx, ny, alvo, tile;

    switch (tecla) {
        case TECLA_CIMA:     case 'w': dy = -1; break;
        case TECLA_BAIXO:    case 's': dy = 1;  break;
        case TECLA_ESQUERDA: case 'a': dx = -1; break;
        case TECLA_DIREITA:  case 'd': dx = 1;  break;
        case ' ': case '.':
            return 1;                                  /* esperar um turno */
        case 'i':
            return inventario_tela(j);
        case '>':
            if (mapa_tile(&j->mapa, h->x, h->y) != TL_DESCE) {
                log_adicionar(&j->log, CINZA, "Não há escada que desce aqui.");
                return 0;
            }
            mundo_ir_para(j, j->profundidade + 1, 1);
            log_adicionar(&j->log, CINZA_ESCURO, "Você desce para o nível %d.",
                          j->profundidade);
            return 0;
        case '<':
            if (mapa_tile(&j->mapa, h->x, h->y) != TL_SOBE) {
                log_adicionar(&j->log, CINZA, "Não há escada que sobe aqui.");
                return 0;
            }
            mundo_ir_para(j, j->profundidade - 1, 0);
            return 0;
        default:
            return 0;
    }

    nx = h->x + dx;
    ny = h->y + dy;
    alvo = ator_em(j, nx, ny);
    tile = mapa_tile(&j->mapa, nx, ny);

    if (alvo >= 0) {
        if (j->atores[alvo].npc >= 0) { aldeia_conversar(j, alvo); return 0; }
        heroi_atacar(j, alvo);
        return 1;
    }
    if (tile == TL_BALCAO) {           /* fala com quem está atrás do balcão */
        alvo = ator_em(j, nx + dx, ny + dy);
        if (alvo >= 0 && j->atores[alvo].npc >= 0) aldeia_conversar(j, alvo);
        return 0;
    }
    if (mapa_bloqueia(&j->mapa, nx, ny)) return 0;

    h->x = nx;
    h->y = ny;
    pegar_itens(j);
    if (tile == TL_DESCE)
        log_adicionar(&j->log, CINZA, "Uma escada desce para a escuridão. (>)");
    if (tile == TL_SOBE)
        log_adicionar(&j->log, CINZA, "Uma escada sobe. (<)");
    return 1;
}
