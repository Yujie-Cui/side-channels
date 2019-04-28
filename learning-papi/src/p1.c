#include <stdio.h>
#include <papi.h>

int main() {

    printf("Hallo Welt!\n");
    
    // Initialize PAPI library
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if(ret != PAPI_VER_CURRENT) {
        printf("Failed to initialize PAPI library.\n");
        return 1;
    }

    return 0;
}
