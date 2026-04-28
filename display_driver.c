#include <stdlib.h>
#include <stdio.h>
#include "pico/stdio.h"
#include <pico/stdlib.h>
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/multicore.h"

// assembled program
#include "hub75.pio.h"

#include "display_driver.h"
#include "debug_utils.h"

// pin definitions
#define RGB_BASE 10
#define R0 10
#define G0 11
#define B0 12
#define R1 13
#define G1 14
#define B1 15
#define SEL_BASE 18
#define SEL0 18
#define SEL1 19
#define SEL2 20
#define SEL3 21
#define LAT_OE_BASE 16
#define LAT 16
#define OE 17
#define CLK 22

// panel definitions
#define ROWS 32
#define COLS 64
#define SCAN_LINES 2
#define ROW_PAIRS ROWS / SCAN_LINES
#define COLOR_DEPTH 10

// frame stuff
uint32_t **frame_buf0;
uint32_t **frame_buf1;
uint32_t **tmp;
uint32_t ***frame_buf_write;
uint32_t ***frame_buf_read;
int done_writing;
int done_reading;
int *done_writing_ptr = &done_writing;
int *done_reading_ptr = &done_reading;
static uint32_t current_row[2][COLS];

// pio stuff
PIO pio;
uint sm_rgb;
uint sm_row;
uint rgb_prog_offs;
uint row_prog_offs;

const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255
};

// gamma correction, adjust brightness (gamma) because led brightness is non-linear
static inline uint32_t gamma_correct(uint32_t pix) {
    return (gamma8[(uint8_t) (pix >> 16)] << 16) | (gamma8[(uint8_t) (pix >> 8)] << 8) | gamma8[(uint8_t) pix];
}

DisplayHandle init_display_driver() {
    // allocate buffer space
    frame_buf0 = calloc(ROWS, sizeof(uint32_t*));
    for (int i = 0; i < ROWS; i ++) {
        frame_buf0[i] = calloc(COLS, sizeof(uint32_t));
    }
    frame_buf1 = calloc(ROWS, sizeof(uint32_t*));
    for (int i = 0; i < ROWS; i ++) {
        frame_buf1[i] = calloc(COLS, sizeof(uint32_t));
    }

    // colors buffers differently, useful to check if the buffer swapping is working correctly
    if (DEBUG_LEVEL >= DEBUG_HIGH) {
        for (int i = 0; i < ROWS; i ++) {
            for (int j = 0; j < COLS; j ++) {
                frame_buf0[i][j] = 0x0000000f;
            }
        }
        for (int i = 0; i < ROWS; i ++) {
            for (int j = 0; j < COLS; j ++) {
                frame_buf1[i][j] = 0x00000f00;
            }
        }
    }

    // initial control values
    frame_buf_read = &frame_buf0;
    frame_buf_write = &frame_buf1;
    done_reading = 0;
    done_writing = 0;

    // pio
    pio = pio0;
    sm_rgb = 0;
    sm_row = 1;
    rgb_prog_offs = pio_add_program(pio, &hub75_data_rgb_program);
    row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb_program_init(pio, sm_rgb, rgb_prog_offs, RGB_BASE, CLK);
    hub75_row_program_init(pio, sm_row, row_prog_offs, SEL_BASE, 4, LAT);

    // start display driver on core 1, leaving core 0 for other tasks
    multicore_launch_core1(start_refresh);

    // create and return display handle
    DisplayHandle dh = {
        .frame_buf_write = frame_buf_write,
        .update_frame = update_frame};
    return dh;
}

// continually refresh the display
void start_refresh() {
    while (1) {
        // if new frame is ready swap buffers
        if (multicore_fifo_rvalid()) {
            uint32_t cmd = multicore_fifo_pop_blocking();
            if (cmd == SWAP) {
                tmp = *frame_buf_read;
                *frame_buf_read = *frame_buf_write;
                *frame_buf_write = tmp;
            }
        }
        // display
        for (int rowsel = 0; rowsel < ROW_PAIRS; ++rowsel) {
            for (int x = 0; x < COLS; ++x) {
                // set current row pixels with gamma correction
                current_row[0][x] = gamma_correct((*frame_buf_read)[rowsel][x]);
                current_row[1][x] = gamma_correct((*frame_buf_read)[rowsel + ROW_PAIRS][x]);
            }
            for (int bit = 0; bit < 8; bit ++) {
                hub75_data_rgb_set_rgb(pio, sm_rgb, rgb_prog_offs, bit);
                for (int x = 0; x < COLS; ++x) {
                    pio_sm_put_blocking(pio, sm_rgb, current_row[0][x]);
                    pio_sm_put_blocking(pio, sm_rgb, current_row[1][x]);
                }
                // 1 dummy pixel per lane
                pio_sm_put_blocking(pio, sm_rgb, 0);
                pio_sm_put_blocking(pio, sm_rgb, 0);
                // sm is finished when it stalls on empty tx fifo
                hub75_wait_tx_stall(pio, sm_rgb);
                // check that previous oen pulse is finished
                hub75_wait_tx_stall(pio, sm_row);
                // latch row data, pulse output enable for new row.
                pio_sm_put_blocking(pio, sm_row, rowsel | (100 * (1 << bit) << 5));
            }
        }
    }
}

// swap new frame into read buffer to be displayed
void update_frame() {
    if (DEBUG_LEVEL >= DEBUG_EXTREME) {
        printf("update_frame\n");
    }
    multicore_fifo_push_blocking(SWAP);
}