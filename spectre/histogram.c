/*
	Generates a histogram of the cache hit/miss threshold	
*/

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#define ARRAY_SIZE 256 // number of possible byte values
#define PAGE_SIZE 4096 // elements in probe_array should be PAGE_SIZE bytes apart to ensure they occupy different cache lines
#define DELTA 1024 // to avoid pre-caching at index 0 * 4096

// histogram data
#define ITER_N 10000
#define MISS 0
#define HIT 1
#define MAX_CYCLES 500
// HIT/MISS -> index: # of cycles to access element ; value: the number of times this cycle count occurred
uint64_t histogram[2][MAX_CYCLES] = {{0}}; 

char secret = 'a';
uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE];

void write_to_file(char* filename) {
	// write data to .csv file
	FILE* fd = fopen(filename, "w");
	if(!fd) {
		printf("Failed to open file.\n");
		return;
	}

	fprintf(fd, "cycles,misses,hits\n");
	for(int i=0; i<MAX_CYCLES; i++) {
		fprintf(fd, "%i,%lu,%lu\n", i, histogram[MISS][i], histogram[HIT][i]);
	}
	
	fclose(fd);
}

int main(int argc, char* argv[]) {
	
	unsigned int junk=0; // holds the signature value of rdtscp() ; not needed
	register uint64_t start, total_cycles; // cycle timestamps
	volatile uint8_t* addr; // pointer to a byte

	// initialize probe_array
	for(int i=0; i<ARRAY_SIZE; i++) {
		probe_array[i * PAGE_SIZE + DELTA] = 1;
	}
	
	for(int op=0; op<2; op++) { // op: MISS/HIT
		for(int iter=0; iter<ITER_N; iter++) {	
			// flush cacheline
			_mm_clflush(&probe_array[secret * PAGE_SIZE + DELTA]);	
			if(op == HIT) probe_array[secret * PAGE_SIZE + DELTA] = 25; // reload the value into the cache	
	
			// time the access	
			addr = &probe_array[secret * PAGE_SIZE + DELTA];
			start = __rdtscp(&junk);
			junk = *addr; // access byte
			total_cycles = __rdtscp(&junk) - start;

			if(total_cycles >= MAX_CYCLES) histogram[op][MAX_CYCLES-1]++;
			else histogram[op][total_cycles]++;
		}
	}

	write_to_file("histogram.csv");
	printf("%-16s %-16s %-16s\n", "cycles", "miss", "hit");
	for(int i=0; i<MAX_CYCLES; i++) {
		printf("%-16i %-16lu %-16lu\n", i, histogram[MISS][i], histogram[HIT][i]);
	}

	return 0;
}
