/*
	Inspired by this paper:
	https://gruss.cc/files/prefetch.pdf	
*/

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#define ARRAY_SIZE 256 // number of possible byte values
#define PAGE_SIZE 4096 // elements in probe_array should be PAGE_SIZE bytes apart to ensure they occupy different cache lines
#define CACHE_HIT_THRESHOLD 100 // cache hit if access time is CACHE_HIT_THRESHOLD cycles or less
#define CACHE_HIT_PREFETCH_THRESHOLD 150

#define NUM_RUNS 300000

uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE];

void flush() {
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE]); // removes from all cache hierarchies
	}
}

// https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
uint64_t rdtsc() {
	uint64_t a, d;
	asm volatile ("mfence");
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	a = (d<<32) | a;
	asm volatile ("mfence");
	return a;
}

void clflush(void* p) {
	asm volatile ("clflush 0(%0)\n"
	  :
	  : "c" (p)
	  : "rax");
}

int main(int argc, char* argv[]) {

    for(int run=0; run<NUM_RUNS; run++) {	
	    register uint64_t start, total_cycles; // cycle timestamps
	    uint8_t* addr; // pointer to a byte
	    char guess = 0;
	    char actual = 7;

	    // remove entire probe_array from cache
	    flush();
	    
	    // access an element
	    probe_array[actual * PAGE_SIZE] = 25;

	    // measure the clflush() instruction for each element
	    for(int i=0; i<ARRAY_SIZE; i++) {
	    	addr = &probe_array[i * PAGE_SIZE];
	    	start = rdtsc();
	    	//_mm_clflush(addr);	
	    	//clflush(addr);
	    	_mm_prefetch(addr, 2);
	    	total_cycles = rdtsc() - start;
	    	if(total_cycles < CACHE_HIT_PREFETCH_THRESHOLD) guess = i;
	    	//if(total_cycles > CACHE_HIT_CLFLUSH1 && total_cycles < CACHE_HIT_CLFLUSH2) guess = i;
	    //	printf("clflush() time for probe_array[%i * %i]: %d cycles\n", i, PAGE_SIZE, (int)total_cycles);
	    }
    }
	//printf("Guess %i, actual %i\n", guess, actual);
		
	return 0;
}
