/*
	Runs two processes ; tests to see if one process can influence branch predictor behavior of the other
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <emmintrin.h> // _mm_clflush__()
#include <x86intrin.h> // __rdtscp()
#include <sched.h> // sched_yield() 

#include "utils.h"

#define NUM_RUNS 50

int test_var = 0; // per-process variable?
int* addr = &test_var;

void measure_func(int num) {	
	unsigned int junk=0; // holds the signature value of rdtscp() ; not needed
	register uint64_t start, total_cycles; // cycle timestamps

	start = __rdtscp(&junk);
	junk = *addr; // access byte
	total_cycles = __rdtscp(&junk) - start;
	printf("%d cycles\n", (int)total_cycles);
	_mm_clflush(addr);	
}

void inject_func(int num) {
	int i = *addr; // access test_var	
	printf("%i\n", getpid());
}

int main(int argc, char* argv[]) {

	pid_t pid;
	user_t user;

	// create a child process
	pid = fork();
		
	if(pid == 0) {
		printf("measure %i\n", getpid());
		user.func = measure_func;
		user.val = 5;
	} else {
		printf("inject %i\n", getpid());
		user.func = inject_func;
		user.val = 10;
	}

	for(int i=0; i<NUM_RUNS; i++) {	
		for(int j=0; j<NUM_RUNS; j++) {
			if(user.val > 0) { // always taken...	
				user.val = user.val + 1;
			}
		}
		_mm_clflush(user.func);
		user.func(0); // influence this branch...
		sched_yield(); // schedule self off the CPU
	}

	if(pid != 0) wait(NULL); // parent waits for child
	printf("%i leaving!\n", getpid());	
	
	return 0;
}
