#include "pico/stdio.h"
#include "display_driver.h"

DisplayHandle dh;

void init_display_utils(DisplayHandle display_handle) {
    dh = display_handle;
}

void test() {
    dh.update_frame();
}