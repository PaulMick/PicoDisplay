#ifndef WIFI_H
#define WIFI_H

#define WIFI_ENABLED 1
#define WIFI_RETRY_TIME_SEC 5
#define WIFI_CHECK_TIME_SEC 20

int init_wifi(int *wifi_connected);
int connect_wifi();
void disconnect_wifi();
int is_wifi_connected();
void set_wifi_status(int status);

#endif