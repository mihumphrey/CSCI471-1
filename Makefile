CC=gcc
CFLAGS=-I.

echo_server: echo_server.o
	$(CC) -o echo_s echo_server.c
