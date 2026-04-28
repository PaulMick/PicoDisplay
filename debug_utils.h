#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

// different debug levels
typedef enum {
    DEBUG_NONE = 0, // no debug prints or displays
    DEBUG_MINIMAL = 1, // minimal debug prints, almost no display differences
    DEBUG_NORMAL = 2, // some debug prints, some display differences
    DEBUG_HIGH = 3, // more debug prints, some display differences
    DEBUG_EXTREME = 4 // maximum debug prints and display differences
} debug_level_t;

#define DEBUG_LEVEL DEBUG_HIGH

#endif