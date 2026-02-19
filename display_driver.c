#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// pin definitions
#define R1 5
#define G1 6
#define B1 7
#define R2 10
#define G2 11
#define B2 12
#define SEL0 14
#define SEL1 15
#define SEL2 16
#define SEL3 17
#define CLK 4
#define LAT 9
#define OE 19

// panel definitions
#define ROW_PAIRS 16
#define COLS 64
#define COLOR_DEPTH 4

void init_display_driver() {
    // gpio
    gpio_init_mask(0xf << 4); // CLK and RGB1
    gpio_init_mask(0xf << 9); // LAT and RGB2
    gpio_init_mask(0xf << 14); // SEL0-3
    gpio_init(19); // OE
    
}