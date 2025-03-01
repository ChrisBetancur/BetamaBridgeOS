#ifndef GPU_H
#define GPU_H
#include <stdint.h>

typedef struct pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;

static const pixel_t PIXEL_WHITE = {0xff, 0xff, 0xff};
static const pixel_t PIXEL_BLACK = {0x00, 0x00, 0x00};

typedef enum {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} color_t;


void gpu_putc(char c);

void framebuffer_set_background(color_t color);

#endif