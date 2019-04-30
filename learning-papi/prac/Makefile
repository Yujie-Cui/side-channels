CC=gcc
CFLAGS=-Wall -O0 -g 
IDIR=-I/usr/local/include
LIBS=/usr/local/lib/libpapi.a

all: p1 hAPI addEvents

%.o: %.c 
	$(CC) $(IDIR) $(LIBS) -c -o $@ $< $(CFLAGS)

p1: p1.o
	$(CC) $(IDIR) p1.o $(LIBS) $(CFLAGS) -o p1

hAPI: hAPI.o
	$(CC) $(IDIR) hAPI.o $(LIBS) $(CFLAGS) -o hAPI

addEvents: addEvents.o
	$(CC) $(IDIR) addEvents.o $(LIBS) $(CFLAGS) -o addEvents

clean:
	rm *.o p1 hAPI addEvents 
