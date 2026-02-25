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
static uint32_t gc_row[2][COLS];

// pio stuff
PIO pio;
uint sm_data;
uint sm_row;
uint data_prog_offs;
uint row_prog_offs;

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

    for (int i = 0; i < ROWS; i ++) {
        for (int j = 0; j < COLS; j ++) {
            frame_buf0[i][j] = 0x000000ff;
        }
    }
    for (int i = 0; i < ROWS; i ++) {
        for (int j = 0; j < COLS; j ++) {
            frame_buf1[i][j] = 0x0000ff00;
        }
    }

    // initial control values
    frame_buf_read = &frame_buf0;
    frame_buf_write = &frame_buf1;
    done_reading = 0;
    done_writing = 0;

    // pio
    pio = pio0;
    sm_data = 0;
    sm_row = 1;
    data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, RGB_BASE, CLK);
    hub75_row_program_init(pio, sm_row, row_prog_offs, SEL_BASE, 4, LAT);

    // start display driver on core 1, leaving core 0 for other tasks
    multicore_launch_core1(start_refresh);

    // create and return display handle
    DisplayHandle dh = {
        .frame_buf_write = frame_buf_write,
        .update_frame = update_frame};
    return dh;

    sleep_ms(1000);

    for (int i = 0; i < ROWS; i ++) {
        for (int j = 0; j < COLS; j ++) {
            frame_buf0[i][j] = 0x00ff0000;
        }
    }
}

// continually refresh the display
void start_refresh() {
    while (1) {
        // if new frame is ready, wait until switched
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
                // set pixels
                gc_row[0][x] = (*frame_buf_read)[rowsel][x];
                gc_row[1][x] = (*frame_buf_read)[rowsel + ROW_PAIRS][x];
            }
            for (int bit = 0; bit < 8; ++bit) {
                hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
                for (int x = 0; x < COLS; ++x) {
                    pio_sm_put_blocking(pio, sm_data, gc_row[0][x]);
                    pio_sm_put_blocking(pio, sm_data, gc_row[1][x]);
                }
                // dummy pixel per lane
                pio_sm_put_blocking(pio, sm_data, 0);
                pio_sm_put_blocking(pio, sm_data, 0);
                // sm is finished when it stalls on empty tx fifo
                hub75_wait_tx_stall(pio, sm_data);
                // also check that previous oen pulse is finished, else things can get out of sequence
                hub75_wait_tx_stall(pio, sm_row);

                // latch row data, pulse output enable for new row.
                pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 5));
            }
        }
    }
}

// swap new frame into read buffer to be displayed
void update_frame() {
    multicore_fifo_push_blocking(SWAP);
}