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
void create_cvs(profile_t* profiles, int num_profiles);

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
        printf("%i: %s\n", i, p->argv[0]);

        pid = fork();
        
        if(pid < 0) {
            perror("Fork error.\n");
            return 1;
        }
        else if(pid == 0) { // child process
            pause(); // wait until events are attached 
            printf("pid=%i: Imma execute %s\n", getpid(), p->argv[0]);
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
                printf("pid=%i: Successfully sent signal to child %i\n", getpid(), pid);
            }

            // sample until child finishes or MAX_SAMPLES is reached 
            p->num_samples = 0;
            while( (wpid = waitpid(pid, &status, WNOHANG)) >= 0 ) {
                if(p->num_samples >= MAX_SAMPLES) { // maximum number of samples reached
                    printf("Reached max samples %i. Stopping.\n", MAX_SAMPLES);
                    kill(pid, SIGTERM); // terminate process
                    sleep(2);
                    kill(pid, SIGKILL);
                    continue;
                }
                // else keep sampling
                if(PAPI_read(EventSet, p->values[p->num_samples]) != PAPI_OK) { // PAPI_accum() saves counter values and resets the counters
                    printf("Failed to record sample %i\n", p->num_samples);
                    continue; // wait until process finishes...
                }
                 
                p->num_samples++;
                printf("Collected %i/%i samples\n", p->num_samples, MAX_SAMPLES); 
                if(wpid == pid) break; // process completed
            }
            if(wpid < 0) {
                printf("Failed to wait for pid=%i\n", pid);
            }
           
            printf("Recorded %i samples for %s\n", p->num_samples, p->argv[0]); 
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
        
        } // parent process ; end

    } // for each profile ; end

    // create CSV file of all data
    create_cvs(profiles, num_profiles);

    return 0;
}

void sig_events_set(int sig) {
    if(sig == SIGUSR1) {
        printf("pid=%i: recieved signal!\n", getpid());
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

void create_cvs(profile_t* profiles, int num_profiles) {
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
        for(int s=0; s<NUM_EVENTS; s++) {
            memset(eventStr, 0, PAPI_MAX_STR_LEN);
            ret = PAPI_event_code_to_name(events[s], eventStr);
            if(ret != PAPI_OK) {
                printf("Failed to convert event %i to string\n", events[s]);
            }
            fprintf(csv, "%s-%s,", basename(p->argv[0]), eventStr); 
        }
    }
    fprintf(csv, "\n");

    // record data of each process ; one sample per row
    for(int i=0; i<MAX_SAMPLES; i++) {
        fprintf(csv, "%i,", i); // sample number
        for(int j=0; j<num_profiles; j++) {
            profile_t* p = &profiles[j];
            for(int s=0; s<NUM_EVENTS; s++) {
                if(i >= p->num_samples) fprintf(csv, "-1,");
                else fprintf(csv, "%llu,", p->values[i][s]);
            }
        }
        fprintf(csv, "\n");
    }

    fclose(csv);
}

