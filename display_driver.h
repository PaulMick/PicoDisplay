#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#define SWAP 67

#define ROWS 32
#define COLS 64

typedef struct {
    uint32_t ***frame_buf_write;
    void (*update_frame)(void);
} DisplayHandle;

DisplayHandle init_display_driver();

void start_refresh();

void update_frame();

#endif