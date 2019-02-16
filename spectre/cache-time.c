/*
	Syracuse SEED Lab
	http://www.cis.syr.edu/~wedu/seed/Labs_16.04/System/Spectre_Attack/Spectre_Attack.pdf	
*/

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#define ARRAY_SIZE 10
#define PAGE_SIZE 4096 // elements in probe_array should be PAGE_SIZE bytes apart to ensure they occupy different cache lines

uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE];

int main(int argc, char* argv[]) {
	
	unsigned int junk=0; // holds the signature value of rdtscp() ; not needed
	// 'register' keyword: hint to the compiler to place variable into a register (not guarenteed)
	register uint64_t start, total_cycles; // cycle timestamps
	volatile uint8_t* addr; // pointer to a byte

	// initialize probe_array
	for(int i=0; i<ARRAY_SIZE; i++) {
		probe_array[i * PAGE_SIZE] = 1;
	}

	// flush probe_array from CPU cache
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE]); // removes from all cache hierarchies
	}

	// access an element
	probe_array[7 * PAGE_SIZE] = 25;
	
	// time each access
	for(int i=0; i<ARRAY_SIZE; i++) {
		addr = &probe_array[i * PAGE_SIZE];
		start = __rdtscp(&junk);
		junk = *addr; // access byte
		//*addr = 'a'; // does not work... interesting
		total_cycles = __rdtscp(&junk) - start;
		printf("Access time probe_array[%i * %i]: %d cycles\n", i, PAGE_SIZE, (int)total_cycles);
	}

	return 0;
}
