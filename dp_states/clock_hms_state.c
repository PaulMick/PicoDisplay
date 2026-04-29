#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdio.h"

#include "assets.h"
#include "gen_utils.h"
#include "display_utils.h"
#include "clock_hms_state.h"
#include "http/http_weather.h"

clock_hms_state_t *state;
struct tm *tm_ptr;
time_t t;

void init_clock_hms_state() {
    state = malloc(sizeof(clock_hms_state_t));
    state->day_abbr[0] = 'N';
    state->day_abbr[1] = 'U';
    state->day_abbr[2] = 'L';
    state->month = 0;
    state->month_day = 0;
    state->current_temp_f = 0;
    state->current_wind_mph = 99;
    state->gust_wind_mph = 99;
    state->weather_code = 0;
    state->hour = 0;
    state->minute = 0;
    state->second = 0;
    state->wifi_connected = 0;
}

static void draw_hm_digit(uint8_t digit, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    switch (digit) {
        case 0:
            draw_rect(x, y, 11, 24, 2, r, g, b);
            return;
        case 1:
            draw_rect(x + 9, y, 2, 24, -1, r, g, b);
            draw_line(x + 7, y, LINE_DOWN, 2, r, g, b);
            draw_line(x + 8, y, LINE_DOWN, 2, r, g, b);
            return;
        case 2:
            draw_rect(x, y, 11, 2, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            draw_rect(x, y + 22, 11, 2, -1, r, g, b);
            draw_rect(x + 9, y + 2, 2, 8, -1, r, g, b);
            draw_rect(x, y + 12, 2, 10, -1, r, g, b);
            return;
        case 3:
            draw_rect(x, y, 11, 2, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            draw_rect(x, y + 22, 11, 2, -1, r, g, b);
            draw_rect(x + 9, y + 2, 2, 8, -1, r, g, b);
            draw_rect(x + 9, y + 12, 2, 10, -1, r, g, b);
            return;
        case 4:
            draw_rect(x, y, 2, 10, -1, r, g, b);
            draw_rect(x + 9, y, 2, 24, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            return;
        case 5:
            draw_rect(x, y, 11, 2, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            draw_rect(x, y + 22, 11, 2, -1, r, g, b);
            draw_rect(x, y + 2, 2, 8, -1, r, g, b);
            draw_rect(x + 9, y + 12, 2, 10, -1, r, g, b);
            return;
        case 6:
            draw_rect(x, y, 11, 2, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            draw_rect(x, y + 22, 11, 2, -1, r, g, b);
            draw_rect(x, y + 2, 2, 8, -1, r, g, b);
            draw_rect(x + 9, y + 12, 2, 10, -1, r, g, b);
            draw_rect(x, y + 12, 2, 10, -1, r, g, b);
            return;
        case 7:
            draw_rect(x, y, 9, 2, -1, r, g, b);
            draw_rect(x + 9, y, 2, 24, -1, r, g, b);
            return;
        case 8:
            draw_rect(x, y, 11, 24, 2, r, g, b);
            draw_rect(x + 2, y + 10, 9, 2, -1, r, g, b);
            return;
        case 9:
            draw_rect(x, y, 11, 2, -1, r, g, b);
            draw_rect(x, y + 10, 11, 2, -1, r, g, b);
            draw_rect(x, y + 22, 11, 2, -1, r, g, b);
            draw_rect(x, y + 2, 2, 8, -1, r, g, b);
            draw_rect(x + 9, y + 12, 2, 10, -1, r, g, b);
            draw_rect(x + 9, y + 2, 2, 8, -1, r, g, b);
            return;
        default: return;
    }
}

void draw_clock_hms_state() {
    // update
    t = time(NULL);
    tm_ptr = localtime(&t);
    state->hour = tm_ptr->tm_hour;
    state->minute = tm_ptr->tm_min;
    state->second = tm_ptr->tm_sec;
    state->month = tm_ptr->tm_mon;
    state->month_day = tm_ptr->tm_mday;
    state->day_abbr[0] = day_abbrs[tm_ptr->tm_wday][0];
    state->day_abbr[1] = day_abbrs[tm_ptr->tm_wday][1];
    state->day_abbr[2] = day_abbrs[tm_ptr->tm_wday][2];
    if (!MILITARY_TIME && state->hour > 12) {
        state->hour -= 12;
    }

    // draw
    char tmp[50];
    // top row
    draw_str(0, 0, state->day_abbr, 3, FONT_5X5_FLEX, 255, 255, 255);
    sprintf(tmp, "%*d", 2, state->month);
    draw_str(16, 0, tmp, 2, FONT_5X5_FLEX, 255, 255, 255);
    draw_str(25, 0, "/", 1, FONT_5X5_FLEX, 255, 255, 255);
    sprintf(tmp, "%-2d", state->month_day);
    draw_str(30, 0, tmp, 2, FONT_5X5_FLEX, 255, 255, 255);
    sprintf(tmp, "%*d", 3, state->current_temp_f);
    draw_str(40, 0, tmp, 3, FONT_5X5_FLEX, 255, 255, 255);
    draw_str(52, 0, "*", 1, FONT_5X5_FLEX, 255, 255, 255);
    // right column
    sprintf(tmp, "%02d", state->second);
    draw_str(54, 7, tmp, 2, FONT_5X5_FLEX, 255, 255, 255);
    sprintf(tmp, "C%*d", 2, state->current_wind_mph);
    draw_str(52, 13, tmp, 3, FONT_5X5_FLEX, 255, 255, 255);
    sprintf(tmp, "G%*d", 2, state->gust_wind_mph);
    draw_str(51, 19, tmp, 3, FONT_5X5_FLEX, 255, 255, 255);
    // TODO: add weather code and wifi connection symbols
    if (state->wifi_connected) {
        draw_img(58, 25, wifi_connected_width, wifi_connected_height, wifi_connected_img);
    } else {
        draw_img(58, 25, wifi_disconnected_width, wifi_disconnected_height, wifi_disconnected_img);
    }
    // main time
    if (HOUR_LEADING_ZERO || state->hour >= 10) {
        draw_hm_digit(state->hour / 10, 1, 7, 255, 255, 255);
    }
    draw_hm_digit(state->hour % 10, 13, 7, 255, 255, 255);
    draw_line(25, 16, LINE_DOWN, 2, 255, 255, 255);
    draw_line(25, 19, LINE_DOWN, 2, 255, 255, 255);
    draw_hm_digit(state->minute / 10, 27, 7, 255, 255, 255);
    draw_hm_digit(state->minute % 10, 39, 7, 255, 255, 255);
}

void deinit_clock_hms_state() {
    free(state);
}
