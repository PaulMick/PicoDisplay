#include "pico/stdio.h"
#include "display_driver.h"
#include "gen_utils.h"

DisplayHandle dh;

// set display handle
void init_display_utils(DisplayHandle display_handle) {
    dh = display_handle;
}

// encode rgb values to xbgr
uint32_t rgb_encode(uint8_t r, uint8_t g, uint8_t b) {
    return (b << 16) | (g << 8) | r;
}

// safely draw non-encoded pixel
void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) {
        return;
    }
    (*dh.frame_buf_write)[y][x] = rgb_encode(r, g, b);
}

// safely draw encoded pixel
void draw_pixel_enc(int x, int y, uint32_t xbgr) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) {
        return;
    }
    (*dh.frame_buf_write)[y][x] = xbgr;
}

// unsafely non-encoded draw pixel (doesn't check frame bounds)
void draw_pixel_raw(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    (*dh.frame_buf_write)[y][x] = rgb_encode(r, g, b);
}

// unsafely draw encoded pixel (doesn't check frame bounds)
void draw_pixel_raw_enc(int x, int y, uint32_t xbgr) {
    (*dh.frame_buf_write)[y][x] = xbgr;
}

// fill entire frame with one color
void fill_frame(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t xbgr = rgb_encode(r, g, b);
    for (int y = 0; y < ROWS; y ++) {
        for (int x = 0; x < COLS; x ++) {
            draw_pixel_raw_enc(x, y, xbgr);
        }
    }
}

// draw pixel-wide line from point, axis, and length
void draw_line(int x, int y, line_dir_t dir, int len, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t xbgr = rgb_encode(r, g, b);
    if (dir == DOWN) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x, y + i, xbgr);
        }
    } else if (dir ==  UP) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x, y - i, xbgr);
        }
    } else if (dir ==  LEFT) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x - i, y, xbgr);
        }
    } else if (dir ==  RIGHT) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x + i, y, xbgr);
        }
    }
}