CC=gcc
CFLAGS=-Wall -O0 
#IDIR=-I/usr/local/include
IDIR=-I/home/atsuko/side-channels/learning-papi/papi/src 
LIBS=/usr/local/lib/libpapi.a

all: p1 hAPI 

%.o: %.c 
	$(CC) $(IDIR) $(LIBS) -c -o $@ $< $(CFLAGS)

p1: p1.o
	$(CC) $(IDIR) p1.o $(LIBS) $(CFLAGS) -o p1

hAPI: hAPI.o
	$(CC) $(IDIR) hAPI.o $(LIBS) $(CFLAGS) -o hAPI

clean:
	rm *.o p1 hAPI
