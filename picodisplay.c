#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"

#include "secrets/wifi_secrets.h"

#include "gen_utils.h"
#include "display_driver.h"
#include "ntp.h"
#include "display_utils.h"
#include "assets.h"
#include "debug_utils.h"

#include <time.h>

#include "hardware/clocks.h"

int init() {
    // Add this before cyw43_arch_init()

    // stdio
    stdio_init_all();

    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("start init\n");
    }

    // wifi
    int init_code = cyw43_arch_init();
    if (init_code) {
        if (DEBUG_LEVEL >= DEBUG_MINIMAL) {
            fprintf(stderr, "Wi-Fi init failed, code %d\n", init_code);
        }
    } else {
        if (DEBUG_LEVEL >= DEBUG_NORMAL) {
            printf("Wi-Fi init successful\n");
        }
    }
    cyw43_arch_enable_sta_mode();
    int retries = 0;
    while (retries < 3) {
        if (DEBUG_LEVEL >= DEBUG_HIGH) {
            printf("Try %d: ", retries + 1);
        }
        int connect_code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, WIFI_AUTH, 10000);
        if (connect_code) {
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                fprintf(stderr, "Timed out connecting to network \"%s\", code %d\n", WIFI_SSID, connect_code);
            }
        } else {
            if (DEBUG_LEVEL >= DEBUG_HIGH) {
                printf("Connected to %s\n", WIFI_SSID);
            }
            break;
        }
        retries ++;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // ntp
    init_ntp();

    // display driver
    DisplayHandle dh = init_display_driver();

    // display utils
    init_display_utils(dh);

    return 0;
}

int run() {
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