#ifndef WIFI_H
#define WIFI_H

#define WIFI_ENABLED 1
#define WIFI_RETRY_TIME_SEC 5
#define WIFI_CHECK_TIME_SEC 20

int init_wifi(int *wifi_connected);
int connect_wifi();
int is_wifi_connected();
void start_check_connect_wifi();

#endif