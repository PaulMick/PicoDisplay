#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#define DMA_CHANNEL_PIXEL 0
#define DMA_CHANNEL_ROW 1

void row_finished_handler();
void init_display_driver();

#endif