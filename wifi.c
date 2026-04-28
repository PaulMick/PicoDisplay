#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "hardware/timer.h"

#include "debug_utils.h"
#include "secrets/wifi_secrets.h"
#include "wifi.h"

int init_wifi() {
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
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    hw_set_bits(&timer1_hw->inte, 1 << 3);
    irq_set_exclusive_handler(timer_hardware_alarm_get_irq_num(timer1_hw, 3), start_check_connect_wifi);
    irq_set_enabled(timer_hardware_alarm_get_irq_num(timer1_hw, 3), true);
    return init_code;
}

int connect_wifi() {
    int connect_code = -67;
    connect_code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, WIFI_AUTH, 5000);
    if (connect_code) {
        if (DEBUG_LEVEL >= DEBUG_NORMAL) {
            fprintf(stderr, "Timed out connecting to network \"%s\", code %d\n", WIFI_SSID, connect_code);
        }
    } else {
        if (DEBUG_LEVEL >= DEBUG_HIGH) {
            printf("Connected to %s\n", WIFI_SSID);
        }
    }
    return connect_code;
}

int is_wifi_connected() {
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    switch (status) {
        case CYW43_LINK_DOWN:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link Down\n");
            }
            return 0;
        case CYW43_LINK_JOIN:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link Joining\n");
            }
            return 1;
        case CYW43_LINK_NOIP:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link No IP\n");
            }
            return 1;
        case CYW43_LINK_UP:
            if (DEBUG_LEVEL >= DEBUG_HIGH) {
                printf("WiFi Status: Link Up\n");
            }
            return 1;
        case CYW43_LINK_FAIL:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link Fail\n");
            }
            return 0;
        case CYW43_LINK_NONET:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link No Network\n");
            }
            return 0;
        case CYW43_LINK_BADAUTH:
            if (DEBUG_LEVEL >= DEBUG_NORMAL) {
                printf("WiFi Status: Link Bad Auth\n");
            }
            return 0;
        default: return 0;
    }
}

void start_check_connect_wifi() {
    hw_clear_bits(&timer1_hw->intr, 1 << 3);
    if (!WIFI_ENABLED) {
        return;
    }
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("start_check_connect_wifi\n");
    }
    if (!is_wifi_connected()) {
        connect_wifi();
        uint64_t target = timer1_hw->timerawl + WIFI_RETRY_TIME_SEC * 1000000;
        timer_hardware_alarm_set_target(timer1_hw, 3, target);
    } else {
        uint64_t target = timer1_hw->timerawl + WIFI_CHECK_TIME_SEC * 1000000;
        timer_hardware_alarm_set_target(timer1_hw, 3, target);
    }
}