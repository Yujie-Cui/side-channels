CC=gcc
CFLAGS=-Wall -march=native -O0
DEPS=

all: test spectre erik-spectre bpoison-spectre prefetch-spectre 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: test.o
	$(CC) test.o $(CFLAGS) -o test

spectre: spectre.o
	$(CC) spectre.o $(CFLAGS) -o spectre

erik-spectre: erik-spectre.o
	$(CC) erik-spectre.o $(CFLAGS) -o erik-spectre

bpoison-spectre: bpoison-spectre.o
	$(CC) bpoison-spectre.o $(CFLAGS) -o bpoison-spectre

prefetch-spectre: prefetch-spectre.o
	$(CC) prefetch-spectre.o $(CFLAGS) -o prefetch-spectre

clean:
	rm *.o test spectre erik-spectre bpoison-spectre prefetch-spectre
