CC = gcc
CFLAGS = -g

all:
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
	$(CC) $(CFLAGS) main.c -o main -ldl
main:
	$(CC) $(CFLAGS) main.c -o main -ldl
library:
	$(CC) $(CFLAGS) library.c -o library.so -fPIC -shared
clean:
	rm main
