#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <libgen.h> // basename()
#include <papi.h>

#include "profiler.h"

/*
    Profile HPC data of process
*/

// signal handler ; lets child know that the child can start exec()
void sig_events_set(int sig);

int parse_config(char* filename, profile_t** profiles);
void print_profiles(profile_t* profiles, int num_profiles);
void create_csv(profile_t* profiles, int num_profiles);
void record_run(profile_t* profiles, int num_profiles);

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("./profile <.config>\n");
        return 1;
    }

    profile_t* profiles = NULL;
    int num_profiles = parse_config(argv[1], &profiles);
    //print_profiles(profiles, num_profiles);

    int EventSet = PAPI_NULL;
    
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

    // add events to EventSet
    ret = PAPI_add_events(EventSet, events, NUM_EVENTS);
    if(ret != PAPI_OK) {
        printf("Failed to add events to EventSet. %i\n", ret);
        return 1;
    }

    // set signal handler
    signal(SIGUSR1, sig_events_set);

    // fork and profile each process
    for(int i=0; i<num_profiles; i++) {
        
        // reset counters
        ret = PAPI_reset(EventSet);
        if(ret != PAPI_OK) {
            printf("Failed to reset counters for iter %i\n", i);
        }

        profile_t* p = &profiles[i];
        pid_t pid, wpid;
        int status = 0;
        long long values[NUM_EVENTS]; // not used ; only used to stop the counters at the end
        printf("%i: Will profile %s\n", i, p->argv[0]);

        pid = fork();
        
        if(pid < 0) {
            perror("Fork error.\n");
            return 1;
        }
        else if(pid == 0) { // child process
            pause(); // wait until events are attached 
            ret = execve(p->argv[0], p->argv, NULL); // execute command line args
            printf("Should never arrive here... execve() error occurred.\n");
        } else { // parent process ; responsible for attaching HPC events to process
            
            // attach events to forked process
            ret = PAPI_attach(EventSet, pid);
            if(ret != PAPI_OK) {
                printf("pid=%i: Failed to attach events to process %i. %i\n", getpid(), pid, ret);
                // add an error handler
                break;
            }

            // start counting 
            if(PAPI_start(EventSet) != PAPI_OK) {
                printf("Failed to start counting\n");
                return 1;
            }

            // send signal to child to start executing
            sleep(1); // this reduces a chance of a race condition ; gives time for child to get to pause() ; must be a better way...
            ret = kill(pid, SIGUSR1);
            if(ret < 0) {
                printf("Failed to send signal to child %i\n", pid);
            } else {
                if(DEBUG) printf("pid=%i: Successfully sent signal to child %i\n", getpid(), pid);
            }

            // sample until child finishes or MAX_SAMPLES is reached 
            p->num_samples = 0;
            long long start_usec = PAPI_get_real_usec();
            while( (wpid = waitpid(pid, &status, WNOHANG)) >= 0 ) {
                if(wpid == pid) break; // process completed
                if(p->num_samples >= MAX_SAMPLES) { // maximum number of samples reached
                    // should terminate process...
                    continue;
                }
                // else keep sampling
                if(PAPI_accum(EventSet, p->values[p->num_samples]) != PAPI_OK) { // PAPI_accum() saves counter values and resets the counters
                    printf("Failed to record sample %i\n", p->num_samples);
                    continue; // wait until process finishes...
                }
                p->timestamps[p->num_samples] = PAPI_get_real_usec() - start_usec;
                if(p->num_samples > 0) p->intervals[p->num_samples] = p->timestamps[p->num_samples] - p->timestamps[p->num_samples - 1];
                else p->intervals[p->num_samples] = p->timestamps[p->num_samples];
                // wait some time, somehow...
                ret = usleep(SAMPLE_FREQ); 
                if(ret < 0) {
                    printf("Failed to sleep for %i microseconds.\n", SAMPLE_FREQ);
                }
                p->num_samples++;
                if(DEBUG) printf("%i/%i samples collected.\n", p->num_samples, MAX_SAMPLES);
            }
            if(wpid < 0) {
                printf("Failed to wait for pid=%i\n", pid);
            }
           
            printf("%i: Recorded %i/%i samples for %s\n", i, p->num_samples, MAX_SAMPLES, p->argv[0]); 
            // stop counters
            if(PAPI_stop(EventSet, values) != PAPI_OK) {
                printf("Failed to stop counters.\n");
                return 1;
            }
 
            // detach events from child process
            ret = PAPI_detach(EventSet);
            if(ret != PAPI_OK) {
                printf("Failed to detach events from process. %i\n", ret);
            }
            printf("---\n");
 
        } // parent process ; end

    } // for each profile ; end

    record_run(profiles, num_profiles);
    create_csv(profiles, num_profiles);

    return 0;
}

void sig_events_set(int sig) {
    if(sig == SIGUSR1) {
        if(DEBUG) printf("pid=%i: recieved signal!\n", getpid());
    }
}

// returns number of processs to profile
int parse_config(char* filename, profile_t** profiles) {
    FILE* config = fopen(filename, "r");
    if(!config) {
        printf("Failed to open %s.\n", filename);
        return 0;
    }

    // get number of processs to profile
    int num_profiles = 0;
    for(char c=getc(config); c!=EOF; c=getc(config)) {
        if (c == '\n') {
            num_profiles++;
        }
    }  
    assert(*profiles == NULL);
    *profiles = (profile_t*) malloc(num_profiles * sizeof(profile_t));
   
    // go to beginning of file
    rewind(config);

    // parse config file
    int process_id = 0;
    while(!feof(config)) {
        char* line = NULL;	
		size_t n = 0;
	
		if(getline(&line, &n, config) > 0) {
            line[strcspn(line, "\r\n")] = 0; // remove the new line
            profile_t* profile = &(*profiles)[process_id];

            profile->argv[0] = strtok(line, " "); // get first argument
            profile->argc = 0;
            while(profile->argv[profile->argc] != NULL && profile->argc < MAX_CMD_ARGS) {
                profile->argc++;
                profile->argv[profile->argc] = strtok(NULL, " ");
            }
        
            process_id++;
        } // if getline() ; end

    } // while reading config ; end
    fclose(config);

    assert(num_profiles == process_id); 
    return num_profiles;    
}

void print_profiles(profile_t* profiles, int num_profiles) {
    printf("%i processs to profile:\n--\n", num_profiles);
    for(int i=0; i<num_profiles; i++) {
        printf("%i: ", i);
        profile_t* profile = &profiles[i];
        // go through command line args
        for(int c=0; c<profile->argc; c++) {
            printf("%s ", profile->argv[c]);
        }
        printf("\n");
    }
}

void create_csv(profile_t* profiles, int num_profiles) {
    FILE* csv = fopen("hpc-data.csv", "w");
    if(!csv) {
        printf("Failed to create .csv file.\n");
        return;
    }
    
    // create csv header
    fprintf(csv, "sample,");
    for(int i=0; i<num_profiles; i++) {
        profile_t* p = &profiles[i];
        char eventStr[PAPI_MAX_STR_LEN];
        int ret;
        //fprintf(csv, "%s-timestamp (usec),", basename(p->argv[0]));
        fprintf(csv, "interval (usec)-%s,", basename(p->argv[0]));
        for(int s=0; s<NUM_EVENTS; s++) {
            memset(eventStr, 0, PAPI_MAX_STR_LEN);
            ret = PAPI_event_code_to_name(events[s], eventStr);
            if(ret != PAPI_OK) {
                printf("Failed to convert event %i to string\n", events[s]);
            }
            fprintf(csv, "%s-%s,", eventStr, basename(p->argv[0])); 
        }
    }
    fprintf(csv, "\n");

    // record data of each process ; one sample per row
    for(int i=0; i<MAX_SAMPLES; i++) {
        fprintf(csv, "%i,", i); // sample number
        for(int j=0; j<num_profiles; j++) {
            profile_t* p = &profiles[j];
            //fprintf(csv, "%llu,", p->timestamps[i]); 
            fprintf(csv, "%llu,", p->intervals[i]);
            for(int s=0; s<NUM_EVENTS; s++) {
                if(i >= p->num_samples) fprintf(csv, "0,");
                else fprintf(csv, "%llu,", p->values[i][s]);
            }
        }
        fprintf(csv, "\n");
    }

    fclose(csv);
}

void record_run(profile_t* profiles, int num_profiles) {
    FILE* run = fopen("run.csv", "w");
    if(!run) {
        printf("Failed to create a run.out file.\n");
    }
    fprintf(run, "Max command args,%i\n", MAX_CMD_ARGS);
    fprintf(run, "Sampling frequency (microsec),%i\n", SAMPLE_FREQ);
    fprintf(run, "Total sample time per program (microsec),%i\n", TOTAL_SAMPLE_TIME);
    fprintf(run, "Max samples per program,%i\n", MAX_SAMPLES);
    fprintf(run, "Number events,%i\n", NUM_EVENTS);
   
    // record events
    char eventStr[PAPI_MAX_STR_LEN];
    int ret;
    fprintf(run, "Events,");
    for(int i=0; i<NUM_EVENTS; i++) {
        memset(eventStr, 0, PAPI_MAX_STR_LEN);
        ret = PAPI_event_code_to_name(events[i], eventStr);
        if(ret != PAPI_OK) {
            printf("Failed to convert event %i to string\n", events[i]);
        }
        fprintf(run, "%s,", eventStr); 
    }
    fprintf(run, "\n");

    // record the programs that ran
    fprintf(run, "Number of programs,%i\n", num_profiles);
    for(int i=0; i<num_profiles; i++) {
        profile_t* p = &profiles[i];
        for(int c=0; c<p->argc; c++) {
            fprintf(run, "%s,", p->argv[c]);
        }
        fprintf(run, "\n");
    }
 
    fclose(run);
}
