CC = gcc
CFLAGS = -g -ldl -lssl -lcrypto

.PHONY: all server
all: server

server:
	$(CC) $(CFLAGS) server.c -o server
clean:
	rm server
