#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#define DMA_CHANNEL_PIXEL 0
#define DMA_CHANNEL_ROW 1
#define DMA_CHANNEL_ROW_FINISHED 2

void init_display_driver();

void row_finished_handler();
void pixel_finished_handler();

#endif