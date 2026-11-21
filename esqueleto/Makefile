# Makefile — esqueleto de um jogo novo.
# Uso: "make" compila; "make limpar" apaga o executável.
# Ao copiar a pasta para o seu projeto, TROQUE o nome do executável
# e acrescente os seus arquivos .c e os módulos do motor que usar.
MOTOR  = ../motor
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -I$(MOTOR)
NOME   = jogo
FONTES = main.c \
         $(MOTOR)/term.c $(MOTOR)/tela.c $(MOTOR)/ui.c $(MOTOR)/rng.c \
         $(MOTOR)/mapa.c $(MOTOR)/dados.c

$(NOME): $(FONTES)
	$(CC) $(CFLAGS) $(FONTES) -o $(NOME) -lm

limpar:
	rm -f $(NOME) $(NOME).exe
