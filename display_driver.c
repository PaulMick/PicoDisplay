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
#define COLOR_DEPTH 8

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
    dma_channel_config_t pixel_channel_cfg = {.ctrl = (
        1 << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB | // chain to row channel when done
        DREQ_PIO0_TX0 << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB | // pace transfer rate with PIO0 tx fifo
        0x2 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB // word-size transfers (32 bits)
    )};
    dma_channel_configure(DMA_CHANNEL_PIXEL, &pixel_channel_cfg, &(pio0_hw->txf), frame_buf0[0], COLS, true);
    dma_channel_config_t row_channel_cfg = {.ctrl = (
        0x2 << DMA_CH1_CTRL_TRIG_DATA_SIZE_LSB // word-size transfers (32 bits)
    )};
    dma_channel_configure(DMA_CHANNEL_ROW, &row_channel_cfg, &(pio0_hw->txf), &oepulse_row, 1, false);
    
    // irq
    irq_set_exclusive_handler(PIO0_IRQ_0, row_finished_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
}

void pixel_finished_handler() {
    printf("pixel finished handlers\n");
}

void row_finished_handler() {
    printf("row finished handler\n");

    // acknowledge interrupt
    pio_interrupt_clear(pio, 0);

    // increment row
    row ++;
    if (row == ROW_PAIRS) {
        row = 0;
        bitplane ++;
        if (bitplane == COLOR_DEPTH) {
            bitplane = 0;
        }
        hub75_pixel_set_shift(pio, sm_pixel, pixel_prog_offset, bitplane);
    }
    oepulse_row = 1 << bitplane + 5 | row << 0;
    dma_channel_set_read_addr(DMA_CHANNEL_PIXEL, frame_buf0[row], true);
    
}