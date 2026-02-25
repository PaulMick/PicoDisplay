#include "pico/stdio.h"
#include "display_driver.h"

DisplayHandle dh;

void init_display_utils(DisplayHandle display_handle) {
    dh = display_handle;
}

uint32_t rgb_encode(uint8_t r, uint8_t g, uint8_t b) {
    return (b << 16) | (g << 8) | r;
}

void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) {
        return;
    }
    *(dh.frame_buf_write)[y][x] = rgb_encode(r, g, b);
}