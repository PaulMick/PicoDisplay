#include <stdio.h>

#include "picodisplay.h"

int main()
{
    int init_success = init();
    if (init_success != 0) {
        fprintf(stderr, "Init Failure: %d\n", init_success);
    }
    int run_success = run();
    return run_success;
}
