/*
 * tela.h — a tela virtual do motor "Make in C".
 *
 * O jogo nunca escreve direto no terminal. Ele desenha numa grade de
 * TELA_L x TELA_A células guardada na memória e, uma vez por quadro,
 * chama tela_mostrar(), que envia ao terminal apenas o que mudou.
 */
#ifndef TELA_H
#define TELA_H

#include <stddef.h>

#define TELA_L 80   /* colunas */
#define TELA_A 24   /* linhas  */

/* As 16 cores do modo texto do PC, na ordem original do DOS. */
enum {
    PRETO, AZUL, VERDE, CIANO, VERMELHO, MAGENTA, MARROM, CINZA,
    CINZA_ESCURO, AZUL_CLARO, VERDE_CLARO, CIANO_CLARO,
    VERMELHO_CLARO, MAGENTA_CLARO, AMARELO, BRANCO
};

/* Uma célula: um glifo (código Unicode) e duas cores. */
typedef struct {
    unsigned int  glifo;
    unsigned char frente;
    unsigned char fundo;
} Celula;

/* ---- Glifos clássicos do PC (código CP437 entre parênteses) ---- */
#define G_ROSTO         0x263Au  /* ☺ (1)   */
#define G_ROSTO_CHEIO   0x263Bu  /* ☻ (2)   */
#define G_COPAS         0x2665u  /* ♥ (3)   */
#define G_OUROS         0x2666u  /* ♦ (4)   */
#define G_PAUS          0x2663u  /* ♣ (5)   */
#define G_ESPADAS       0x2660u  /* ♠ (6)   */
#define G_BOLINHA       0x2022u  /* • (7)   */
#define G_CIRCULO       0x25CBu  /* ○ (9)   */
#define G_MASCULINO     0x2642u  /* ♂ (11)  */
#define G_FEMININO      0x2640u  /* ♀ (12)  */
#define G_NOTA          0x266Au  /* ♪ (13)  */
#define G_NOTAS         0x266Bu  /* ♫ (14)  */
#define G_SOL           0x263Cu  /* ☼ (15)  */
#define G_TRI_DIR       0x25BAu  /* ► (16)  */
#define G_TRI_ESQ       0x25C4u  /* ◄ (17)  */
#define G_SETA_CIMA     0x2191u  /* ↑ (24)  */
#define G_SETA_BAIXO    0x2193u  /* ↓ (25)  */
#define G_SETA_DIR      0x2192u  /* → (26)  */
#define G_SETA_ESQ      0x2190u  /* ← (27)  */
#define G_TRI_CIMA      0x25B2u  /* ▲ (30)  */
#define G_TRI_BAIXO     0x25BCu  /* ▼ (31)  */
#define G_CASA          0x2302u  /* ⌂ (127) */
#define G_A_TREMA       0x00C4u  /* Ä (142) */
#define G_O_TREMA       0x00D6u  /* Ö (153) */
#define G_LIBRA         0x00A3u  /* £ (156) */
#define G_SOMBRA_LEVE   0x2591u  /* ░ (176) */
#define G_SOMBRA_MEDIA  0x2592u  /* ▒ (177) */
#define G_SOMBRA_FORTE  0x2593u  /* ▓ (178) */
#define G_BLOCO         0x2588u  /* █ (219) */
#define G_MEIO_BAIXO    0x2584u  /* ▄ (220) */
#define G_MEIO_ESQ      0x258Cu  /* ▌ (221) */
#define G_MEIO_DIR      0x2590u  /* ▐ (222) */
#define G_MEIO_CIMA     0x2580u  /* ▀ (223) */
#define G_ALFA          0x03B1u  /* α (224) */
#define G_PI            0x03C0u  /* π (227) */
#define G_SIGMA         0x03A3u  /* Σ (228) */
#define G_TAU           0x03C4u  /* τ (231) */
#define G_PHI           0x03A6u  /* Φ (232) */
#define G_TETA          0x0398u  /* Θ (233) */
#define G_OMEGA         0x03A9u  /* Ω (234) */
#define G_DELTA         0x03B4u  /* δ (235) */
#define G_INFINITO      0x221Eu  /* ∞ (236) */
#define G_FI            0x03C6u  /* φ (237) */
#define G_TRIPLO        0x2261u  /* ≡ (240) */
#define G_MAIS_MENOS    0x00B1u  /* ± (241) */
#define G_APROX         0x2248u  /* ≈ (247) */
#define G_GRAU          0x00B0u  /* ° (248) */
#define G_PONTO         0x00B7u  /* · (250) */
#define G_RAIZ          0x221Au  /* √ (251) */
#define G_QUADRADO      0x25A0u  /* ■ (254) */

/* Molduras simples */
#define G_H             0x2500u  /* ─ (196) */
#define G_V             0x2502u  /* │ (179) */
#define G_SE            0x250Cu  /* ┌ (218) superior esquerdo */
#define G_SD            0x2510u  /* ┐ (191) superior direito  */
#define G_IE            0x2514u  /* └ (192) inferior esquerdo */
#define G_ID            0x2518u  /* ┘ (217) inferior direito  */
#define G_T_ESQ         0x251Cu  /* ├ (195) */
#define G_T_DIR         0x2524u  /* ┤ (180) */
#define G_T_CIMA        0x252Cu  /* ┬ (194) */
#define G_T_BAIXO       0x2534u  /* ┴ (193) */
#define G_CRUZ          0x253Cu  /* ┼ (197) */
/* Molduras duplas */
#define G_H2            0x2550u  /* ═ (205) */
#define G_V2            0x2551u  /* ║ (186) */
#define G_SE2           0x2554u  /* ╔ (201) */
#define G_SD2           0x2557u  /* ╗ (187) */
#define G_IE2           0x255Au  /* ╚ (200) */
#define G_ID2           0x255Du  /* ╝ (188) */
#define G_T_ESQ2        0x2560u  /* ╠ (204) */
#define G_T_DIR2        0x2563u  /* ╣ (185) */
#define G_T_CIMA2       0x2566u  /* ╦ (203) */
#define G_T_BAIXO2      0x2569u  /* ╩ (202) */
#define G_CRUZ2         0x256Cu  /* ╬ (206) */

/* ---- Desenho ---- */
void    tela_limpar(void);
void    tela_por(int x, int y, unsigned int glifo, int frente, int fundo);
Celula *tela_celula(int x, int y);          /* NULL se (x,y) estiver fora */
int     tela_texto(int x, int y, int frente, int fundo, const char *texto);
int     tela_textof(int x, int y, int frente, int fundo, const char *formato, ...);
int     tela_texto_max(int x, int y, int frente, int fundo,
                       const char *texto, int max_colunas);
void    tela_texto_centro(int y, int frente, int fundo, const char *texto);
void    tela_retangulo(int x, int y, int l, int a,
                       unsigned int glifo, int frente, int fundo);
void    tela_moldura(int x, int y, int l, int a, int dupla,
                     int frente, int fundo);

/* ---- Envio ao terminal ---- */
void    tela_mostrar(void);
void    tela_redesenhar_tudo(void);

/* ---- Guardar e recuperar a tela (para janelas por cima do jogo) ---- */
void    tela_guardar(void);
void    tela_recuperar(void);

/* ---- Texto UTF-8 ---- */
unsigned int utf8_ler(const char **texto);          /* lê 1 caractere   */
int          utf8_escrever(unsigned int cp, char *destino); /* devolve bytes */
int          utf8_largura(const char *texto);       /* nº de caracteres */
void         utf8_ajustar(char *destino, size_t tamanho,
                          const char *texto, int colunas);
unsigned int tela_cp437(int codigo);                /* código DOS -> Unicode */

#endif
