CC=gcc
CFLAGS=-Wall -march=native
DEPS=utils.h

all: cache-time flush-reload flush-prefetch spec-exec spectre bpoison

%.o: %.c $(DEPS)
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

bpoison: bpoison.o utils.o
	$(CC) bpoison.o utils.o $(CFLAGS) -no-pie -g -o bpoison

clean:
	rm *.o cache-time flush-reload flush-prefetch spec-exec spectre bpoison
