CC=gcc
CFLAGS=-Wall -march=native -O0 -g
DEPS=utils.h

all: cache-time flush-reload flush-prefetch spec-exec spectre bpoison bpoison2 histogram

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

bpoison2: bpoison2.o utils.o
	$(CC) bpoison2.o utils.o $(CFLAGS) -no-pie -g -o bpoison2

histogram: histogram.o
	$(CC) histogram.o $(CFLAGS) -no-pie -g -o histogram

clean:
	rm *.o cache-time flush-reload flush-prefetch spec-exec spectre bpoison bpoison2 histogram
