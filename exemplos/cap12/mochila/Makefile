# Makefile do exemplo da mochila.
# A linha com "mochila:" diz: para fazer 'mochila', preciso destes .o;
# a linha de baixo (começando com TAB!) diz COMO fazer.
CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -g

mochila: main.o mochila.o
	$(CC) main.o mochila.o -o mochila

main.o: main.c mochila.h
	$(CC) $(CFLAGS) -c main.c

mochila.o: mochila.c mochila.h
	$(CC) $(CFLAGS) -c mochila.c

limpar:
	rm -f *.o mochila mochila.exe
