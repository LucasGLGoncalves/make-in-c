# Makefile — A Coroa de Ferro
MOTOR  = ../motor
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -I$(MOTOR)
FONTES = main.c dados_jogo.c mundo.c heroi.c monstros.c aldeia.c \
         desenho.c salvar.c \
         $(MOTOR)/term.c $(MOTOR)/tela.c $(MOTOR)/ui.c $(MOTOR)/rng.c \
         $(MOTOR)/mapa.c $(MOTOR)/dados.c $(MOTOR)/fov.c $(MOTOR)/caminho.c \
         $(MOTOR)/gerador.c $(MOTOR)/dialogo.c

coroa: $(FONTES) coroa.h
	$(CC) $(CFLAGS) $(FONTES) -o coroa

limpar:
	rm -f coroa coroa.exe coroa.sav
