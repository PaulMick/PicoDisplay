#ifndef NTP_H
#define NTP_H

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <time.h>
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define TIMEZONE_OFFSET_SEC (-5 * 3600) // offset from UTC in seconds

typedef struct NTP_T_ {
    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;
    async_at_time_worker_t request_worker;
    async_at_time_worker_t resend_worker;
} NTP_T;

NTP_T* init_ntp();

#endif