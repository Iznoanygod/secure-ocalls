CC = gcc
CFLAGS = -g -lssl -lcrypto -ldl

.PHONY: all main library
all: main library

main:
	$(CC) main.c -o main $(CFLAGS)
library:
	$(CC) library.c -o library.so $(CFLAGS) -shared -fPIC -O0
clean:
	rm main library.so
