CC=gcc
CFLAGS=-Wall -march=native -O0
DEPS=

all: toy 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

toy: toy.o
	$(CC) toy.o $(CFLAGS) -o toy

clean:
	rm *.o toy 
