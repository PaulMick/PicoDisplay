#include <stdio.h>

#include "picodisplay.h"
#include "debug_utils.h"

int main()
{
    int init_success = init();
    if (init_success != 0) {
        if (DEBUG_LEVEL >= DEBUG_MINIMAL) {
            fprintf(stderr, "Init Failure: %d\n", init_success);
        }
    }
    int run_success = run();
    return run_success;
}
