#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

typedef enum {
    DEBUG_NONE,
    DEBUG_MINIMAL,
    DEBUG_NORMAL,
    DEBUG_HIGH,
    DEBUG_EXTREME
} debug_level_t;

#define DEBUG_LEVEL DEBUG_EXTREME

#endif