CC=gcc
CFLAGS=-Wall -O0 -g 
IDIR=-I/usr/local/include
LIBS=/usr/local/lib/libpapi.a

all: profiler

%.o: %.c 
	$(CC) $(IDIR) $(LIBS) -c -o $@ $< $(CFLAGS)

profiler: profiler.o
	$(CC) $(IDIR) profiler.o $(LIBS) $(CFLAGS) -o hpc-profiler

clean:
	rm *.o hpc-profiler
