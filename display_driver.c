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
#define ROW_PAIRS 16
#define ROWS 32
#define COLS 64
#define SCAN_LINES 2
#define COLOR_DEPTH 8

// frame stuff
uint32_t **frame_buf0;
uint32_t **frame_buf1;
int done_writing;
int done_reading;
int read_buf_num;
int write_buf_num;

// for pio state
int row;

void init_display_driver() {
    // allocate buffer space
    // frame_buf0 = calloc(ROWS, sizeof(uint32_t*));
    // for (int i = 0; i < ROWS; i ++) {
    //     frame_buf0[i] = calloc(COLS, sizeof(uint32_t));
    // }
    // frame_buf1 = calloc(ROWS, sizeof(uint32_t*));
    // for (int i = 0; i < ROWS; i ++) {
    //     frame_buf1[i] = calloc(COLS, sizeof(uint32_t));
    // }

    // // initial control values
    // done_reading = 0;
    // done_writing = 0;
    // read_buf_num = 0;
    // write_buf_num = 1;
    // row = 0;

    // gpio
    // gpio_init_mask(0x3f << RGB_BASE); // rgb0 and rgb1
    // gpio_init_mask(0xf << SEL_BASE); // sel0-3
    gpio_init(R0);
    gpio_init(G0);
    gpio_init(B0);
    gpio_init(R1);
    gpio_init(G1);
    gpio_init(B1);
    gpio_init(SEL0);
    gpio_init(SEL1);
    gpio_init(SEL2);
    gpio_init(SEL3);
    gpio_init(CLK); // clk
    gpio_init(LAT); // lat
    gpio_init(OE); // oe
    // gpio_set_dir_out_masked(0x3f << RGB_BASE);
    // gpio_set_dir_out_masked(0xf << SEL_BASE);
    gpio_set_dir(R0, 1);
    gpio_set_dir(G0, 1);
    gpio_set_dir(B0, 1);
    gpio_set_dir(R1, 1);
    gpio_set_dir(G1, 1);
    gpio_set_dir(B1, 1);
    gpio_set_dir(SEL0, 1);
    gpio_set_dir(SEL1, 1);
    gpio_set_dir(SEL2, 1);
    gpio_set_dir(SEL3, 1);
    gpio_set_dir(CLK, 1);
    gpio_set_dir(LAT, 1);
    gpio_set_dir(OE, 1);
    while (1) {
        printf("test loop\n");
        // gpio_put(R0, 0);
        // gpio_put(G0, 0);
        // gpio_put(B0, 0);
        // gpio_put(R1, 0);
        // gpio_put(R1, 0);
        // gpio_put(R1, 0);
        // gpio_put(SEL0, 0);
        // gpio_put(SEL1, 0);
        // gpio_put(SEL2, 0);
        // gpio_put(SEL3, 0);
        // gpio_put(CLK, 0);
        // gpio_put(LAT, 0);
        // gpio_put(OE, 1);
        // sleep_ms(100);

        gpio_put(R0, 1);
        gpio_put(G0, 1);
        gpio_put(B0, 1);
        gpio_put(R1, 1);
        gpio_put(G1, 1);
        gpio_put(B1, 1);
        gpio_put(OE, 0);
        for (int i = 0; i < ROWS / 2; i ++) {
            gpio_put_masked(0xf << SEL_BASE, row << SEL_BASE);
            // gpio_put(SEL0, 0);
            // gpio_put(SEL1, 0);
            // gpio_put(SEL2, 0);
            // gpio_put(SEL3, 0);
            row ++;
            if (row == ROW_PAIRS) {
                row = 0;
            }
            gpio_put(OE, 0);
            for (int j = 0; j < COLS; j ++) {
                gpio_put(CLK, 1);
                gpio_put(CLK, 0);
            }
            gpio_put(LAT, 1);
            gpio_put(LAT, 0);
            sleep_ms(1);
            gpio_put(OE, 1);
            
        }
        gpio_put(OE, 1);

    }

    // pio
    // PIO pio = pio0;
    // uint sm_pixel = 0;
    // uint sm_row = 1;
    // uint pixel_prog_offset = pio_add_program(pio, &hub75_pixel_program);
    // uint row_prog_offset = pio_add_program(pio, &hub75_row_program);
    // hub75_pixel_program_init(pio, pixel_prog_offset, sm_pixel, RGB_BASE, CLK);
    // hub75_row_program_init(pio, row_prog_offset, sm_row, SEL_BASE, LAT_OE_BASE);

    // dma
    // dma_channel_config_t pixel_channel_cfg = {.ctrl = (
    //     DREQ_PIO0_TX0 << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB | // pace transfer rate with PIO0 tx fifo
    //     0x2 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB // word-size transfers (32 bits)
    // )};
    // dma_channel_configure(DMA_CHANNEL_PIXEL, &pixel_channel_cfg, pio0_hw->rxf, frame_buf0[0], COLS, true);
    
    //test
}

void row_finished_handler() {
    // acknowledge interrupt
    dma_hw->ints0 = 1 << DMA_CHANNEL_ROW;

}