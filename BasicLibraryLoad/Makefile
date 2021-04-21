CC = gcc
CFLAGS = -g

all: main library

main:
	$(CC) $(CFLAGS) main.c -o main -ldl
library:
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
clean:
	rm main library.so
