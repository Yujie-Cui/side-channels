#include <stdio.h>
#include <papi.h>

int main() {

    printf("Ich lerne uber PAPI\n");
    
    // initialize PAPI library
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if(ret != PAPI_VER_CURRENT) {
        printf("Failed to initialize PAPI library.\n");
        return 1;
    }

    //// create an event set
    //int eventSet = PAPI_NULL;
    //if(PAPI_create_eventset(&eventSet) != PAPI_OK) {
    //    printf("Failed to create an event set.\n");
    //    return 1;
    //}

    // check to see if the preset, PAPI_TOT_INS, exists 
    if(PAPI_query_event(PAPI_TOT_INS) != PAPI_OK) {
        printf("PAPI_TOT_INS preset event does not exist.\n");
        return 1;
    } 

    // get details about the preset, PAPI_TOT_INS
    //PAPI_preset_info_t info;
    PAPI_event_info_t info;
    if(PAPI_get_event_info(PAPI_TOT_INS, &info) != PAPI_OK) {
        printf("Failed to extract details of preset event.\n");
        return 1;
    }

    if(info.count > 0) {
        printf("PAPI_TOT_INS is available on this CPU!\n");
    } 

    return 0;
}
