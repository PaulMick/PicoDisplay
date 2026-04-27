#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"

#include "secrets/wifi.h"

#include "gen_utils.h"
#include "display_driver.h"
#include "display_utils.h"
#include "assets.h"

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
    while (1) {
        fill_frame(0, 0, 0);
        ///////////////////////////

        

        ///////////////////////////
        sleep_ms(1);
        update_frame();
        sleep_ms(9);
    }
}