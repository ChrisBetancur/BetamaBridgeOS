#include <common/stdio.h>
#include <kernel/uart.h>

void putc(unsigned char c)
{
    while ( mmio_read(UART0_FR) & (1 << 5) ) { }
    mmio_write(UART0_DR, c);
}

void puthex(uint32_t value)
{
    const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        putc(hex_chars[(value >> (i * 4)) & 0xF]); // Extract and print hex digit
    }
}


unsigned char getc()
{
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

void puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        putc((unsigned char)str[i]);
}