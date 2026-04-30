#ifndef HTTP_WEATHER_H
#define HTTP_WEATHER_H

#include "lwip/apps/http_client.h"

typedef struct _WEATHER_HTTP_REQUEST_T {
    const char *hostname;
    const char *url;
    httpc_headers_done_fn headers_fn;
    altcp_recv_fn recv_fn;
    httpc_result_fn result_fn;
    void *callback_arg;
    uint16_t port;
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    struct altcp_tls_config *tls_config;
    altcp_allocator_t tls_allocator;
#endif
    httpc_connection_t settings;
    int complete;
    httpc_result_t result;
} WEATHER_HTTP_REQUEST_T;

typedef struct _weather_t {
    int current_temp_f;
    int current_wind_speed_mph;
    int gust_wind_speed_mph;
} weather_t;

weather_t get_weather();

#endif