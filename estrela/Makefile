# Makefile — Estrela Errante
MOTOR  = ../motor
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -I$(MOTOR)
FONTES = main.c universo.c nave.c voo.c estacao.c salvar.c \
         $(MOTOR)/term.c $(MOTOR)/tela.c $(MOTOR)/ui.c $(MOTOR)/rng.c \
         $(MOTOR)/mapa.c $(MOTOR)/dados.c $(MOTOR)/dialogo.c

estrela: $(FONTES) estrela.h
	$(CC) $(CFLAGS) $(FONTES) -o estrela -lm

limpar:
	rm -f estrela estrela.exe estrela.sav
