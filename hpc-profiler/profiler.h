#ifndef PROFILE_H
#define PROFILE_H

#define DEBUG 0

#define MAX_CMD_ARGS 256
//#define SAMPLE_FREQ 100000 // in microseconds ; 100000 microseconds = 100 miliseconds
#define SAMPLE_FREQ 1000 // in microseconds ; 100000 microseconds = 100 miliseconds
//#define TOTAL_SAMPLE_TIME 60000000 // in microseconds ; 6e7 microseconds = 60 seconds
#define TOTAL_SAMPLE_TIME 6000000 // in microseconds ; 6e7 microseconds = 60 seconds
#define MAX_SAMPLES (TOTAL_SAMPLE_TIME / SAMPLE_FREQ) // 100 miliseconds frequency and 60 second sample time = 600 samples

#define NUM_EVENTS 3
int events[NUM_EVENTS] = {PAPI_TOT_INS, PAPI_L3_TCM, PAPI_L3_TCA};

// holds HPC data for each thread
typedef struct profile_t {
    int argc;
    char* argv[MAX_CMD_ARGS]; // command line arguments of thread
    int num_samples;
    long long values[MAX_SAMPLES][NUM_EVENTS]; // HPC data
    long long timestamps[MAX_SAMPLES];
    long long intervals[MAX_SAMPLES];
    float avr_time_betw_samples;
} profile_t;

#endif // PROFILE_H
