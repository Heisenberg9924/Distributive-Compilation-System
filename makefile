CC = gcc
CFLAGS = -Wall -pthread

all: client server 

client: client.c
	$(CC) $(CFLAGS) client.c -o client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

