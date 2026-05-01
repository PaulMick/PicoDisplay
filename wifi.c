#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "hardware/timer.h"

#include "debug_utils.h"
#include "secrets/wifi_secrets.h"
#include "wifi.h"

int *wifi_connected;

int init_wifi(int *wifi_con_in) {
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("init_wifi\n");
    }
    wifi_connected = wifi_con_in;
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
    cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);
    return init_code;
}

int connect_wifi() {
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("connect_wifi\n");
    }
    int connect_code = -67;
    connect_code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, WIFI_AUTH, 5000);
    if (connect_code == 0) {
        *wifi_connected = 1;
        if (DEBUG_LEVEL >= DEBUG_HIGH) {
            printf("Connected to %s\n", WIFI_SSID);
        }
    } else {
        *wifi_connected = 0;
        if (DEBUG_LEVEL >= DEBUG_NORMAL) {
            fprintf(stderr, "Timed out connecting to network \"%s\", code %d\n", WIFI_SSID, connect_code);
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
                printf("WiFi Status: Link Joined (Connected)\n");
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

void disconnect_wifi() {
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        printf("disconnect_wifi\n");
    }
    *wifi_connected = 0;
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    sleep_ms(100);
}

void set_wifi_status(int status) {
    *wifi_connected = status;
}