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
#define DELTA 1024 // to avoid pre-caching at index 0 * 4096
#define CACHE_HIT_THRESHOLD 100 // cache hit if access time is CACHE_HIT_THRESHOLD cycles or less

char secret = 'a';
char guess = '?';
uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE];

void reload() {
	
	unsigned int junk=0; // holds the signature value of rdtscp() ; not needed
	register uint64_t start, total_cycles; // cycle timestamps
	volatile uint8_t* addr; // pointer to a byte

	for(int i=0; i<ARRAY_SIZE; i++) {
		addr = &probe_array[i * PAGE_SIZE + DELTA];
		start = __rdtscp(&junk);
		junk = *addr; // access byte
		total_cycles = __rdtscp(&junk) - start;
		if(total_cycles < CACHE_HIT_THRESHOLD) guess = i;
	}

}

void flush() {
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]); // removes from all cache hierarchies
	}
}

void victim() {
	probe_array[secret * PAGE_SIZE + DELTA] = 5;
}

int main(int argc, char* argv[]) {
		
	// initialize probe_array
	for(int i=0; i<ARRAY_SIZE; i++) {
		probe_array[i * PAGE_SIZE + DELTA] = 1;
	}

	// flush probe_array from CPU cache	
	flush();

	// victim accesses an element
	victim();
	
	// reloads probe_array elements and times each access
	reload();
	
	printf("Guess: %c, acutal secret: %c\n", guess, secret);

	return 0;
}
