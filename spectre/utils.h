#ifndef UTILS_H
#define UTILS_H

/*
	utility functions for performing side-channel attacks
*/

#include <stdint.h>

#define PAGE_SIZE 4096

// https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
uint64_t rdtsc();
void clflush(void* p);

#endif // UTILS_H
