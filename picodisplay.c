#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"

#include "secrets/wifi.h"

#include "display_driver.h"
#include "display_utils.h"

// debugging
#include "hardware/pio.h"

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
    init_display_driver();

    // display utils
    init_display_utils();
}

int run() {
    while (1) {
        // printf("Hello World!\n");
        // int pixel_tx_empty = pio_sm_is_tx_fifo_empty(pio0, 0);
        // int pixel_rx_empty = pio_sm_is_rx_fifo_empty(pio0, 0);
        // int pixel_tx_full = pio_sm_is_tx_fifo_full(pio0, 0);
        // int pixel_rx_full = pio_sm_is_rx_fifo_full(pio0, 0);
        // int pixel_stat = pio_sm_is_exec_stalled(pio0, 0);
        // int row_tx_empty = pio_sm_is_tx_fifo_empty(pio0, 1);
        // int row_rx_empty = pio_sm_is_rx_fifo_empty(pio0, 1);
        // int row_tx_full = pio_sm_is_tx_fifo_full(pio0, 1);
        // int row_rx_full = pio_sm_is_rx_fifo_full(pio0, 1);
        // int row_stat = pio_sm_is_exec_stalled(pio0, 1);
        // printf("pixel: stat %d, tx full %d, tx empty %d, rx full %d, rx empty %d\n", pixel_stat, pixel_tx_full, pixel_tx_empty, pixel_rx_full, pixel_rx_empty);
        // printf("row: stat %d, tx full %d, tx empty %d, rx full %d, rx empty %d\n", row_stat, row_tx_full, row_tx_empty, row_rx_full, row_rx_empty);
        // int pixel_pc = pio_sm_get_pc(pio0, 0);
        // int row_pc = pio_sm_get_pc(pio0, 1);
        // printf("pc: pixel %d, row %d\n", pixel_pc, row_pc);
        // uint32_t pixel_instr = pio0->instr_mem[pixel_pc];
        // printf("%ld\n");
        printf("hello\n");
        sleep_ms(1000);
    }
}