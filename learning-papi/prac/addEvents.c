#include <stdio.h>
#include <papi.h>

/*
    Adding events to an EventSet
*/

int main() {

    int EventSet = PAPI_NULL;
    long long values[3];   

    // initialize PAPI library
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if(ret != PAPI_VER_CURRENT) {
        printf("Failed to initialize PAPI library.\n");
        return 1;
    }

    // create an EventSet
    if(PAPI_create_eventset(&EventSet) != PAPI_OK) {
        printf("Failed to create an EventSet.\n");
        return 1;
    } 

    // Add event to EventSet
    if(PAPI_add_event(EventSet, PAPI_TOT_INS) != PAPI_OK) {
        printf("Failed to add PAPI_TOT_INS to EventSet.\n");
        return 1;
    }

    // Try adding another event
    if(PAPI_add_event(EventSet, PAPI_L3_TCM) != PAPI_OK) {
        printf("Failed to add PAPI_L3_TCM to EventSet.\n");
        return 1;
    }

    // ...and another event 
    if(PAPI_add_event(EventSet, PAPI_L3_TCA) != PAPI_OK) {
        printf("Failed to add PAPI_L3_TCA to EventSet.\n");
        return 1;
    }

    // could have also used PAPI_add_events()

    /*
        Use die counters !
    */ 

    if(PAPI_start(EventSet) != PAPI_OK) {
        printf("Failed to start counting\n");
        return 1;
    }

    // some computation here
    printf("Some computation here.\n");

    // read values
    if(PAPI_read(EventSet, values) != PAPI_OK) {
        printf("Failed to read counters.\n");
        return 1;
    } 

    // how do I read these values...? Like this?
    printf("%25s %25s %25s\n", "Total insn", "Total L3 misses", "Total L3 accesses");
    printf("%25llu %25llu %25llu\n", values[0], values[1], values[2]);

    if(PAPI_stop(EventSet, values) != PAPI_OK) {
        printf("Failed to stop counters.\n");
        return 1;
    }

    return 0;
}
