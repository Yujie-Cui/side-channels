CC=gcc
CFLAGS=-Wall -march=native -O0
DEPS=

all: spectre erik-spectre

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

spectre: spectre.o
	$(CC) spectre.o $(CFLAGS) -o spectre

erik-spectre: erik-spectre.o
	$(CC) erik-spectre.o $(CFLAGS) -o erik-spectre

clean:
	rm *.o spectre erik-spectre
