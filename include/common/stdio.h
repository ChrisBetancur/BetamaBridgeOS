#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>

void putc(unsigned char c);

void puthex(uint32_t value);

unsigned char getc();

void puts(const char* str);

void putdec(uint32_t value);

void printf(const char *format, ...);

size_t strlen(const char* str);

int strcmp(const char *s1, const char *s2);

char* strcat(char* dest, const char* src);

char** split_str(const char input[], char delimiter, int max_tokens);

#endif