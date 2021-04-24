CC = gcc
CFLAGS = -g -ldl -lssl -lcrypto

.PHONY: all server
all: server

server:
	$(CC) server.c -o server $(CFLAGS)
clean:
	rm server
