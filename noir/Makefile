# Makefile — Porto Sombrio
MOTOR  = ../motor
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -I$(MOTOR)
FONTES = main.c locais.c caso.c desenho.c \
         $(MOTOR)/term.c $(MOTOR)/tela.c $(MOTOR)/ui.c $(MOTOR)/rng.c \
         $(MOTOR)/mapa.c $(MOTOR)/dados.c $(MOTOR)/dialogo.c

noir: $(FONTES) noir.h
	$(CC) $(CFLAGS) $(FONTES) -o noir

limpar:
	rm -f noir noir.exe
