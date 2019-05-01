CC=gcc
CFLAGS=-Wall -march=native -O0
DEPS=utils.h

all: spectre 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

spectre: spectre.o
	$(CC) spectre.o $(CFLAGS) -o spectre

clean:
	rm *.o spectre
