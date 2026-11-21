# Makefile — O Templo da Serpente
# Uso: "make" compila; "make limpar" apaga o executável.
MOTOR  = ../motor
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -I$(MOTOR)
FONTES = main.c niveis.c jogador.c monstros.c desenho.c recordes.c \
         $(MOTOR)/term.c $(MOTOR)/tela.c $(MOTOR)/ui.c $(MOTOR)/rng.c \
         $(MOTOR)/mapa.c $(MOTOR)/dados.c

templo: $(FONTES) templo.h
	$(CC) $(CFLAGS) $(FONTES) -o templo

limpar:
	rm -f templo templo.exe
