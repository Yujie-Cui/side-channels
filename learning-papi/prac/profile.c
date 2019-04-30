#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <papi.h>

#include "profile.h"

/*
    Profile HPC data of thread
*/

int parse_config(char* filename, profile_t** profiles);
void print_profiles(profile_t* profiles, int num_profiles);

// signal handler ; lets child know that the child can start exec()
void sig_events_set(int sig) {
    if(sig == SIGUSR1) {
        printf("pid=%i: recieved signal!\n", getpid());
    }
}

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
    int status = 0; // status of child process
    pid_t wpid; // return value of waitpid()

    // fork and profile each thread
    for(int i=0; i<num_profiles; i++) {
        profile_t* p = &profiles[i];
        pid_t pid;
        printf("%i: %s\n", i, p->argv[0]);

        if(( pid = fork()) == -1) {
            perror("Fork error.\n");
            return 1;
        }

        // child process
        if(pid == 0) { 
            // wait until events are attached 
            pause();    
            // execute command line args
            printf("pid=%i: I got here! Imma exit now.\n", getpid());
            exit(0);
        } else { // parent process ; responsible for attaching HPC events to thread
            // attach events to forked process
            ret = PAPI_attach(EventSet, pid);
            if(ret != PAPI_OK) {
                printf("pid=%i: Failed to attach events to thread %i. %i\n", getpid(), pid, ret);
                // add an error handler
                break;
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

            printf("pid=%i: Parent got here!\n", getpid());
            
            // detach events
            ret = PAPI_detach(EventSet);
            if(ret != PAPI_OK) {
                printf("Failed to detach events from thread. %i\n", ret);
            }

        } // parent process ; end

    } // for each profile ; end
 
    //if(PAPI_start(EventSet) != PAPI_OK) {
    //    printf("Failed to start counting\n");
    //    return 1;
    //}

    //// some computation here
    //printf("Some computation here.\n");

    //// read values
    //if(PAPI_read(EventSet, values) != PAPI_OK) {
    //    printf("Failed to read counters.\n");
    //    return 1;
    //} 

    //// how do I read these values...? Like this?
    //printf("%25s %25s %25s\n", "Total insn", "Total L3 misses", "Total L3 accesses");
    //printf("%25llu %25llu %25llu\n", values[0], values[1], values[2]);

    //if(PAPI_stop(EventSet, values) != PAPI_OK) {
    //    printf("Failed to stop counters.\n");
    //    return 1;
    //}

    return 0;
}

// returns number of threads to profile
int parse_config(char* filename, profile_t** profiles) {
    FILE* config = fopen(filename, "r");
    if(!config) {
        printf("Failed to open %s.\n", filename);
        return 0;
    }

    // get number of threads to profile
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
    int thread_id = 0;
    while(!feof(config)) {
        char* line = NULL;	
		size_t n = 0;
	
		if(getline(&line, &n, config) > 0) {
            line[strcspn(line, "\r\n")] = 0; // remove the new line
            profile_t* profile = &(*profiles)[thread_id];

            profile->argv[0] = strtok(line, " "); // get first argument
            profile->argc = 0;
            while(profile->argv[profile->argc] != NULL && profile->argc < MAX_CMD_ARGS) {
                profile->argc++;
                profile->argv[profile->argc] = strtok(NULL, " ");
            }
        
            thread_id++;
        } // if getline() ; end

    } // while reading config ; end
    fclose(config);

    assert(num_profiles == thread_id); 
    return num_profiles;    
}

void print_profiles(profile_t* profiles, int num_profiles) {
    printf("%i threads to profile:\n--\n", num_profiles);
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
