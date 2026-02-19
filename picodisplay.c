#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"

int init() {
    // stdio
    stdio_init_all();

    // wifi
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
    }
    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

int run() {
    while (true) {
        printf("Hello World!\n");
        sleep_ms(1000);
    }
}