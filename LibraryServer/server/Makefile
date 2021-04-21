CC = gcc
CFLAGS = -g

all: library server

library:
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
server:
	$(CC) $(CFLAGS) server.c -o server
clean:
	rm server library.so
