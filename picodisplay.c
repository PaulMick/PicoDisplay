#include "pico/stdio.h"

#include "gen_utils.h"
#include "display_driver.h"
#include "ntp.h"
#include "wifi.h"
#include "display_utils.h"
#include "assets.h"
#include "debug_utils.h"

#include <time.h>

#include "hardware/clocks.h"

int wifi_status = -1;

int init() {
    // stdio
    stdio_init_all();

    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("start init\n");
    }

    // wifi
    init_wifi();
    // connect_wifi();
    start_check_connect_wifi();


    // ntp
    init_ntp();

    // display driver
    DisplayHandle dh = init_display_driver();

    // display utils
    init_display_utils(dh);

    return 0;
}

int run() {
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("start run\n");
    }
    struct tm *tm_ptr;
    time_t t;
    while (1) {
        fill_frame(0, 0, 0);
        ///////////////////////////

        t = time(NULL);
        tm_ptr = localtime(&t);
        draw_str(-10, 0, asctime(tm_ptr), FONT_5X5_FLEX, 255, 255, 255);

        ///////////////////////////
        sleep_ms(1);
        update_frame();
        sleep_ms(9);
    }
}