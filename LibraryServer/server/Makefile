CC = gcc
CFLAGS = -g

all:
	$(CC) $(CFLAGS) server.c -o server
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
library:
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
server:
	$(CC) $(CFLAGS) server.c -o server
clean:
	rm server
