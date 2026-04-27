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

#include <time.h>

int init() {
    // stdio
    stdio_init_all();

    printf("start init\n");

    // wifi
    int init_code = cyw43_arch_init();
    if (init_code) {
        fprintf(stderr, "Wi-Fi init failed, code %d\n", init_code);
    } else {
        printf("Wi-Fi init successful\n");
    }
    cyw43_arch_enable_sta_mode();
    int connect_code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, WIFI_AUTH, 10000);
    if (connect_code) {
        fprintf(stderr, "Timed out connecting to network \"%s\", code %d\n", WIFI_SSID, connect_code);
    } else {
        printf("Connected to %s\n", WIFI_SSID);
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // ntp
    init_ntp();

    NTP_T *state = init_ntp();
    if (!state) {
        return 1;
    }
    printf("Press 'q' to quit\n");
    hard_assert(async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),  &state->request_worker, 0)); // make the first request
    while(true) {
        int key = getchar_timeout_us(0);
        if (key == 'q' || key == 'Q') {
            break;
        }
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer interrupt) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(at_the_end_of_time);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    free(state);

    // display driver
    DisplayHandle dh = init_display_driver();

    // display utils
    init_display_utils(dh);
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