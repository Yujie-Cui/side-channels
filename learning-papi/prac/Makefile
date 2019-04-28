CC=gcc
CFLAGS=-Wall -O0 
IDIR=-I/usr/local/include
LIBS=/usr/local/lib/libpapi.a

all: p1 

%.o: %.c 
	$(CC) $(IDIR) $(LIBS) -c -o $@ $< $(CFLAGS)

p1: p1.o
	$(CC) $(IDIR) p1.o $(LIBS) $(CFLAGS) -o p1

clean:
	rm *.o p1
