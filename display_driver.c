#include <stdlib.h>
#include <stdio.h>
#include "pico/stdio.h"
#include <pico/stdlib.h>
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

// assembled program
#include "hub75.pio.h"

#include "display_driver.h"

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
uint16_t **frame_buf0;
uint16_t **frame_buf1;
int done_writing;
int done_reading;
int read_buf_num;
int write_buf_num;

// pio stuff
PIO pio;
uint sm_pixel;
uint sm_row;
uint pixel_prog_offset;
uint row_prog_offset;
uint row;
uint bitplane;
uint32_t oepulse_row;

// misc
uint32_t dummy_pixel_data[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
uint32_t row_finished_data = 0;

void init_display_driver() {
    // allocate buffer space
    frame_buf0 = calloc(ROWS, sizeof(uint16_t*));
    for (int i = 0; i < ROWS; i ++) {
        frame_buf0[i] = calloc(COLS, sizeof(uint16_t));
    }
    frame_buf1 = calloc(ROWS, sizeof(uint16_t*));
    for (int i = 0; i < ROWS; i ++) {
        frame_buf1[i] = calloc(COLS, sizeof(uint16_t));
    }

    for (int i = 0; i < ROWS; i ++) {
        for (int j = 0; j < COLS; j ++) {
            frame_buf0[i][j] = 0xffff;
        }
    }

    // initial control values
    done_reading = 0;
    done_writing = 0;
    read_buf_num = 0;
    write_buf_num = 1;
    row = 0;

    // pio
    PIO pio = pio0;
    uint sm_data = 0;
    uint sm_row = 1;
    uint data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    uint row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, RGB_BASE, CLK);
    hub75_row_program_init(pio, sm_row, row_prog_offs, SEL_BASE, 4, LAT);

    static uint32_t gc_row[2][COLS];

    while (1) {
        for (int rowsel = 0; rowsel < ROW_PAIRS; ++rowsel) {
            for (int x = 0; x < COLS; ++x) {
                // gc_row[0][x] = gamma_correct_565_888(frame_buf0[x][rowsel]);
                // gc_row[1][x] = gamma_correct_565_888(frame_buf0[x][ROW_PAIRS + rowsel]);
                gc_row[0][x] = 0x00ff00ff; // XRGB
                gc_row[1][x] = 0xffffffff;
            }
            for (int bit = 0; bit < 8; ++bit) {
                hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
                for (int x = 0; x < COLS; ++x) {
                    pio_sm_put_blocking(pio, sm_data, gc_row[0][x]);
                    pio_sm_put_blocking(pio, sm_data, gc_row[1][x]);
                }
                // Dummy pixel per lane
                pio_sm_put_blocking(pio, sm_data, 0);
                pio_sm_put_blocking(pio, sm_data, 0);
                // SM is finished when it stalls on empty TX FIFO
                hub75_wait_tx_stall(pio, sm_data);
                // Also check that previous OEn pulse is finished, else things can get out of sequence
                hub75_wait_tx_stall(pio, sm_row);

                // Latch row data, pulse output enable for new row.
                pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 5));
            }
        }
    }
}