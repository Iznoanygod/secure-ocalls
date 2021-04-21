CC = gcc
CFLAGS = -g -lssl -lcrypto -ldl

.PHONY: all main library
all: main library

main:
	$(CC) $(CFLAGS) main.c -o main
library:
	$(CC) $(CFLAGS) library.c -o library.so -shared -fPIC -O0
clean:
	rm main library.so
