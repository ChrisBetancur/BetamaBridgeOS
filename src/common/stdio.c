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

void putdec(uint32_t value) {
    // We'll use a simple algorithm to convert the number into a string.
    char buf[12];  // Enough to hold a 32-bit number in decimal.
    int i = 0;
    
    // Special case: value == 0.
    if (value == 0) {
        putc('0');
        return;
    }
    
    // Convert the number into digits (in reverse order).
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Print digits in the correct order.
    for (int j = i - 1; j >= 0; j--) {
        putc(buf[j]);
    }
}