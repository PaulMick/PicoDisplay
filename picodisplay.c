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
    // int x = 0;
    // int dir = 1;
    while (1) {
        fill_frame(0, 0, 0);
        ///////////////////////////
        // if (x >= 63 && dir == 1) {
        //     dir = -1;

        // } else if (x <= 0 && dir == -1) {
        //     dir = 1;
        // }

        // for (int i = 0; i < 64; i ++) {
        //     draw_line(i, 0, LINE_DOWN, 32, 255 - 4 * i, 0, i * 4);
        // }

        // draw_line(x, 0, LINE_DOWN, 32, 255, 255, 255);

        draw_rect(0, 0, 64, 32, 1, 255, 255, 255);

        draw_str(2, 2, "HELLO WORLD!", FONT_5X5_FLEX, 255, 255, 0);

        
        // x += (x > 1 ? (x / 16) + 1 : 1) * dir;
        ///////////////////////////
        sleep_ms(1);
        update_frame();
        sleep_ms(9);
    }
}