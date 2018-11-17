CC=gcc
FLAGS=-g -Wall -c

servidor:	servidores.o
	$(CC) main.c servidores.o -o servidor.out -lpthread

servidores.o:	src/servidores.c	headers/servidores.h
	$(CC) $(FLAGS)	src/servidores.c

clean:
	rm -rf *.o servidor.out
