#include <stdio.h>
#include <papi.h>

/*
    Tests the high-level API
    http://icl.cs.utk.edu/projects/papi/files/documentation/PAPI_USER_GUIDE.htm#HIGH_LEVEL_EXAMPLE
*/

#define NUM_EVENTS 2

int main() {

    int num_hw_counters = 0; // number of hardware counters that are readable at once
    if((num_hw_counters = PAPI_num_counters()) <= PAPI_OK) {
        printf("Failed to initialize counters.\n");
        return 1;
    }
    printf("%i hardware counters available.\n");
 
    int events[NUM_EVENTS] = {PAPI_TOT_INS, PAPI_TOT_CYC};
    long long values[NUM_EVENTS];

    // start counting events
    if(PAPI_start_counters(events, NUM_EVENTS) != PAPI_OK) {
        printf("Failed to start counting events.\n");
        return 1;
    }

    printf("Profiing printf()\n");

    // read the counters
    if(PAPI_read_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("Failed to read the counters\n");    
    }

    printf("Total insn: %llu\n", values[0]);
    printf("Total cycles: %llu\n", values[1]);

    // add to counters (? not sure what this does)
    if(PAPI_accum_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("Failed to add to counters.\n");
    }
    printf("After adding the counters: %lld\n", values[0]);

    // stop the counters
    if(PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("Failed to stop the counters.\n");
    }

    printf("After stopping the counters: %lld\n", values[0]);

    return 0;
}
