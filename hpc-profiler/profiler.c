#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <papi.h>

#include "profiler.h"

/*
    Profile HPC data of process
*/

// signal handler ; lets child know that the child can start exec()
void sig_events_set(int sig);

int parse_config(char* filename, profile_t** profiles);
void print_profiles(profile_t* profiles, int num_profiles);

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("./profile <.config>\n");
        return 1;
    }

    profile_t* profiles = NULL;
    int num_profiles = parse_config(argv[1], &profiles);
    print_profiles(profiles, num_profiles);

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
        printf("%i: %s\n", i, p->argv[0]);

        pid = fork();
        
        if(pid < 0) {
            perror("Fork error.\n");
            return 1;
        }
        else if(pid == 0) { // child process
            pause(); // wait until events are attached 
            ret = execve(p->argv[0], p->argv, NULL); // execute command line args
            printf("Should not get here... execve() error occurred.\n");
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

            // send signal to child ; ready to execute
            ret = kill(pid, SIGUSR1);
            if(ret < 0) {
                printf("Failed to send signal to child %i\n", pid);
            } else {
                printf("pid=%i: Successfully sent signal to child %i\n", getpid(), pid);
            }

            // wait for child to complete
            while( (wpid = waitpid(pid, &status, WUNTRACED)) > 0) {
                if(wpid == pid) break; 
            }
            if(wpid < 0) {
                printf("Failed to wait for pid=%i\n", pid);
            }
            
            // read values
            if(PAPI_read(EventSet, p->values) != PAPI_OK) {
                printf("Failed to read counters.\n");
                return 1;
            } 
            // stop counters
            if(PAPI_stop(EventSet, p->values) != PAPI_OK) {
                printf("Failed to stop counters.\n");
                return 1;
            }
 
            // detach events
            ret = PAPI_detach(EventSet);
            if(ret != PAPI_OK) {
                printf("Failed to detach events from process. %i\n", ret);
            }
        
            // print out statistics
            printf("%25s %25s %25s\n", "Total insn", "Total L3 misses", "Total L3 accesses");
            printf("%25llu %25llu %25llu\n", p->values[0], p->values[1], p->values[2]);

        } // parent process ; end

    } // for each profile ; end

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
