CC=gcc
CFLAGS=-Wall -march=native -O0
DEPS=

all: spectre erik-spectre bpoison-spectre flush-prefetch-spectre 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

spectre: spectre.o
	$(CC) spectre.o $(CFLAGS) -o spectre

erik-spectre: erik-spectre.o
	$(CC) erik-spectre.o $(CFLAGS) -o erik-spectre

bpoison-spectre: bpoison-spectre.o
	$(CC) bpoison-spectre.o $(CFLAGS) -o bpoison-spectre

flush-prefetch-spectre: flush-prefetch-spectre.o
	$(CC) flush-prefetch-spectre.o $(CFLAGS) -o flush-prefetch-spectre

clean:
	rm *.o spectre erik-spectre bpoison-spectre flush-prefetch-spectre
