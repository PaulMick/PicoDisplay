#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int init() {
    // stdio
    stdio_init_all();

    // wifi
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

int run() {
    while (true) {
        printf("Hello World!\n");
        sleep_ms(1000);
    }
}