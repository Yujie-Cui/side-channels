CC=gcc
CFLAGS=-Wall -march=native

all: cache-time flush-reload flush-prefetch spec-exec spectre

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

cache-time: cache-time.o
	$(CC) cache-time.o $(CFLAGS) -o cache-time 

flush-reload: flush-reload.o
	$(CC) flush-reload.o $(CFLAGS) -o flush-reload

flush-prefetch: flush-prefetch.o
	$(CC) flush-prefetch.o $(CFLAGS) -o flush-prefetch

spec-exec: spec-exec.o
	$(CC) spec-exec.o $(CFLAGS) -o spec-exec

spectre: spectre.o
	$(CC) spectre.o $(CFLAGS) -o spectre

clean:
	rm *.o cache-time flush-reload flush-prefetch spec-exec spectre
