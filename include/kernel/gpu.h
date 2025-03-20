#ifndef GPU_H
#define GPU_H
#include <stdint.h>

#include <kernel/framebuffer.h>


void gpu_putc(char c);

void gpu_puts(const char* str);

void framebuffer_set_background(color_t color);

#endif