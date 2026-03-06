#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include "gen_utils.h"
#include "display_driver.h"

void init_display_utils(DisplayHandle dh);

uint32_t rgb_encode(uint8_t r, uint8_t g, uint8_t b);

void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void draw_pixel_enc(int x, int y, uint32_t xbgr);
void draw_pixel_raw(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void draw_pixel_raw_enc(int x, int y, uint32_t xbgr);
void fill_frame(uint8_t r, uint8_t g, uint8_t b);
void draw_line(int x, int y, line_dir_t dir, int len, uint8_t r, uint8_t g, uint8_t b);
void draw_rect(int x, int y, int width, int height, int thickness, uint8_t r, uint8_t g, uint8_t b);
int draw_char(int x, int y, char c, font_t font, uint8_t r, uint8_t g, uint8_t b);
void draw_str(int x, int y, char *str, font_t font, uint8_t r, uint8_t g, uint8_t b);
void draw_img(int x, int y, img_t img);

#endif