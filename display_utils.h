#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include "display_driver.h"

void init_display_utils(DisplayHandle dh);

uint32_t rgb_encode(uint8_t r, uint8_t g, uint8_t b);

void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

#endif