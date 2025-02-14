#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>

void putc(unsigned char c);

void puthex(uint32_t value);

unsigned char getc();

void puts(const char* str);

#endif