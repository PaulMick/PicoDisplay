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
uint32_t **frame_buf0;
uint32_t **frame_buf1;
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
            frame_buf0[i][j] = 0xffffffff;
        }
    }

    // initial control values
    done_reading = 0;
    done_writing = 0;
    read_buf_num = 0;
    write_buf_num = 1;
    row = 0;

    // pio
    pio = pio0;
    sm_pixel = 0;
    sm_row = 1;
    pixel_prog_offset = pio_add_program(pio, &hub75_pixel_program);
    row_prog_offset = pio_add_program(pio, &hub75_row_program);
    row = 0;
    bitplane = 0;
    oepulse_row = 0;
    hub75_pixel_program_init(pio, pixel_prog_offset, sm_pixel, RGB_BASE, CLK);
    hub75_row_program_init(pio, row_prog_offset, sm_row, SEL_BASE, LAT_OE_BASE);

    // dma
    // pixel
    dma_channel_config cfg_pixel = dma_channel_get_default_config(DMA_CHANNEL_PIXEL);
    channel_config_set_transfer_data_size(&cfg_pixel, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg_pixel, true);
    channel_config_set_write_increment(&cfg_pixel, false);
    uint dreq_pixel = pio_get_dreq(pio, sm_pixel, true);
    channel_config_set_dreq(&cfg_pixel, dreq_pixel);
    channel_config_set_high_priority(&cfg_pixel, true);
    channel_config_set_chain_to(&cfg_pixel, DMA_CHANNEL_DUMMY_PIXEL);
    dma_channel_configure(DMA_CHANNEL_PIXEL, &cfg_pixel, &pio->txf[sm_pixel], NULL, COLS, false);
    // dummy pixel
    dma_channel_config cfg_dummy_pixel = dma_channel_get_default_config(DMA_CHANNEL_DUMMY_PIXEL);
    channel_config_set_transfer_data_size(&cfg_dummy_pixel, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg_dummy_pixel, false);
    channel_config_set_write_increment(&cfg_dummy_pixel, false);
    uint dreq_dummy_pixel = pio_get_dreq(pio, sm_pixel, true);
    channel_config_set_dreq(&cfg_dummy_pixel, dreq_dummy_pixel);
    channel_config_set_high_priority(&cfg_dummy_pixel, true);
    channel_config_set_chain_to(&cfg_dummy_pixel, DMA_CHANNEL_ROW);
    dma_channel_configure(DMA_CHANNEL_DUMMY_PIXEL, &cfg_dummy_pixel, &pio->txf[sm_pixel], NULL, 8, false);
    // row
    dma_channel_config cfg_row = dma_channel_get_default_config(DMA_CHANNEL_ROW);
    channel_config_set_transfer_data_size(&cfg_row, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg_row, false); // maybe different
    channel_config_set_write_increment(&cfg_row, false);
    uint dreq_row = pio_get_dreq(pio, sm_row, true);
    channel_config_set_dreq(&cfg_row, dreq_row);
    channel_config_set_high_priority(&cfg_row, true);
    channel_config_set_chain_to(&cfg_row, DMA_CHANNEL_ROW_FINISHED); // ROW or ROW_FINISHED?
    dma_channel_configure(DMA_CHANNEL_ROW, &cfg_row, &pio->txf[sm_row], NULL, 1, false);
    // row finished
    dma_channel_config cfg_row_finished = dma_channel_get_default_config(DMA_CHANNEL_ROW_FINISHED);
    channel_config_set_transfer_data_size(&cfg_row_finished, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg_row_finished, false);
    channel_config_set_write_increment(&cfg_row_finished, false);
    uint dreq_row_finished = pio_get_dreq(pio, sm_row, false);
    channel_config_set_dreq(&cfg_row_finished, dreq_row_finished);
    channel_config_set_high_priority(&cfg_row_finished, true);
    dma_channel_configure(DMA_CHANNEL_ROW_FINISHED, &cfg_row_finished, &row_finished_data, &pio->rxf[sm_row], 1, false);
    // other setup
    dma_channel_set_read_addr(DMA_CHANNEL_DUMMY_PIXEL, dummy_pixel_data, false);
    dma_channel_set_read_addr(DMA_CHANNEL_ROW, &oepulse_row, false);

    // dma irq
    irq_set_exclusive_handler(DMA_IRQ_0, row_finished_handler);
    dma_channel_set_irq0_enabled(DMA_CHANNEL_ROW_FINISHED, true);
    irq_set_enabled(DMA_IRQ_0, true);

    // start
    dma_channel_set_read_addr(DMA_CHANNEL_PIXEL, frame_buf0[row], true);
}

void row_finished_handler() {
    // clear interrupt
    dma_hw->ints0 = 1 << DMA_CHANNEL_ROW_FINISHED;

    row ++;
    if (row == ROW_PAIRS) {
        row = 0;
        bitplane ++;
        if (bitplane == COLOR_DEPTH) {
            bitplane = 0;
        }
        hub75_pixel_set_shift(pio, sm_pixel, pixel_prog_offset, bitplane);
    }
    oepulse_row = row | (1 << (bitplane + 4));
    dma_channel_set_read_addr(DMA_CHANNEL_PIXEL, frame_buf0[row], true);
}