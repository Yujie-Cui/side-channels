#ifndef PROFILE_H
#define PROFILE_H

#define MAX_CMD_ARGS 256

#define NUM_EVENTS 3
int events[NUM_EVENTS] = {PAPI_TOT_INS, PAPI_L3_TCM, PAPI_L3_TCA};

// holds HPC data for each thread
typedef struct profile_t {
    int argc;
    char* argv[MAX_CMD_ARGS]; // command line arguments of thread
    long long values[NUM_EVENTS]; // HPC data
} profile_t;

#endif // PROFILE_H
