#ifndef UTILS_H
#define UTILS_H

/*
	utility functions for performing side-channel attacks
*/

#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct user_t {
	void (*func)(int); // a pointer to a function that takes in an int as a parameter
	int val;
} user_t;

// https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
uint64_t rdtsc();
void clflush(void* p);

#endif // UTILS_H
