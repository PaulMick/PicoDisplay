#ifndef CLOCK_HMS_STATE_H
#define CLOCK_HMS_STATE_H

#define HOUR_LEADING_ZERO 0

typedef struct _clock_hms_state_t {
    char day_abbr[3];
    int month;
    int month_day;
    int current_temp_f;
    int current_wind_mph;
    int gust_wind_mph;
    int weather_code;
    int hour;
    int minute;
    int second;
    int wifi_connected;
} clock_hms_state_t;

void init_clock_hms_state();
void draw_clock_hms_state();
void deinit_clock_hms_state();

#endif