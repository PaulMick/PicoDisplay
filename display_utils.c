#include "pico/stdio.h"
#include "display_driver.h"
#include "gen_utils.h"
#include "assets.h"
#include "display_utils.h"

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
    if (len < 0) {
        return;
    }
    uint32_t xbgr = rgb_encode(r, g, b);
    if (dir == LINE_DOWN) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x, y + i, xbgr);
        }
    } else if (dir ==  LINE_UP) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x, y - i, xbgr);
        }
    } else if (dir ==  LINE_LEFT) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x - i, y, xbgr);
        }
    } else if (dir ==  LINE_RIGHT) {
        for (int i = 0; i < len; i ++) {
            draw_pixel_enc(x + i, y, xbgr);
        }
    }
}

// draw rect with optional edge thickness (-1 for filled) from point (top left corner), width, and height
void draw_rect(int x, int y, int width, int height, int thickness, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t xbgr = rgb_encode(r, g, b);
    if (thickness == -1) {
        for (int i = 0; i < width; i ++) {
            for (int j = 0; j < height; j ++) {
                draw_pixel_enc(x + i, y + j, xbgr);
            }
        }
    } else {
        for (int i = 0; i < thickness; i ++) {
            draw_line(x + i, y + i, LINE_RIGHT, width - (2 * i), r, g, b);
            draw_line(x + i, y + height - i - 1, LINE_RIGHT, width - (2 * i), r, g, b);
            draw_line(x + i, y + i + 1, LINE_DOWN, height - 2 - (2 * i), r, g, b);
            draw_line(x + width - i - 1, y + i + 1, LINE_DOWN, height - 2 - (2 * i), r, g, b);
        }
    }
}

// draw a character at x, y (top left corner)
int draw_char(int x, int y, char c, font_t font, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t xbgr = rgb_encode(r, g, b);
    if (font == FONT_5X5_FLEX) {
        uint32_t font_char = font_5x5_flex[(int) c];
        int width = font_char >> 25;
        for (int i = 0; i < width; i ++) {
            for (int j = 0; j < 5; j ++) {
                if (font_char & (1 << (i * 5 + j))) {
                    draw_pixel_enc(x + i, y + j, xbgr);
                }
            }
        }
        return width;
    }
    return 0;
}

// draw a string at x, y (top left corner)
void draw_str(int x, int y, char *str, font_t font, uint8_t r, uint8_t g, uint8_t b) {
    int i = 0;
    while (str[i] != '\0') {
        x += draw_char(x, y, str[i++], font, r, g, b) + 1;
    }
}