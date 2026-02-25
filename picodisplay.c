#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"

#include "secrets/wifi.h"

#include "gen_utils.h"
#include "display_driver.h"
#include "display_utils.h"

int init() {
    // stdio
    stdio_init_all();

    // wifi
    if (cyw43_arch_init()) {
        fprintf(stderr, "Wi-Fi init failed\n");
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, WIFI_AUTH, 10000)) {
        fprintf(stderr, "Timed out connecting to network \"%s\"\n", WIFI_SSID);
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // display driver
    DisplayHandle dh = init_display_driver();

    // display utils
    init_display_utils(dh);
}

int run() {
    uint frame_count = 0;
    while (1) {
        printf("frame\n");
        draw_line(1, frame_count, RIGHT, 5, 255, 255, 0);

        update_frame();
        frame_count ++;
        sleep_ms(1000);
    }
}