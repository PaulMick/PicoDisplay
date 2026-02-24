#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#define DMA_CHANNEL_PIXEL 0
#define DMA_CHANNEL_DUMMY_PIXEL 1
#define DMA_CHANNEL_ROW 2
#define DMA_CHANNEL_ROW_FINISHED 3

typedef struct {
    uint32_t ***frame_buf_write;
    void (*update_frame)(void);
} DisplayHandle;

DisplayHandle init_display_driver();

void start_refresh();

void update_frame();

#endif