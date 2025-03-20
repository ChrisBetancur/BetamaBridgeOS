#include <common/stdio.h>
#include <kernel/uart.h>
#include <stdarg.h>
#include <kernel/memory.h>


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

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}


void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);  // Initialize variadic arguments

    for (const char *p = format; *p != '\0'; p++) {
        if (*p == '%') {
            p++;  // Move to the character after '%'
            switch (*p) {
                // Character
                case 'c': {
                    int c = va_arg(args, int);  // Promote char to int
                    putc(c);
                    break;
                }
                // String
                case 's': {
                    char *str = va_arg(args, char*);
                    puts(str);
                    break;
                }
                // Decimal (unsigned, as per your putdec)
                case 'd': {
                    uint32_t value = va_arg(args, uint32_t);
                    putdec(value);
                    break;
                }
                // Hexadecimal
                case 'x': {
                    uint32_t value = va_arg(args, uint32_t);
                    puthex(value);
                    break;
                }
                // Literal '%'
                case '%': {
                    putc('%');
                    break;
                }
                // Unknown specifier: print "%" and the character
                default: {
                    putc('%');
                    putc(*p);
                    break;
                }
            }
        } else {
            // Regular character: print directly
            putc(*p);
        }
    }

    va_end(args);  // Clean up variadic arguments
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char* strcat(char* dest, const char* src) {
    char *p = dest;
    while (*p != '\0') {
        p++;
    }
    
    while ((*p++ = *src++) != '\0') {
        ;
    }
    
    return dest;
}

void int_to_ascii(int n, char *buffer) {
    int i = 0;
    uint32_t is_negative = 0;
    
    if (n < 0) {
        is_negative = 1;
        n = -n;
    }
    
    do {
        buffer[i++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);
    
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    buffer[i] = '\0';
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

char** split_str(const char input[], char delimiter, int max_tokens) {
    int token_count = 0;
    int len = strlen(input);
    
    // Allocate tokens array
    char** tokens = kmalloc(max_tokens * sizeof(char*));
    if (!tokens) return NULL;

    int start = 0;
    int in_token = 0;

    for (int i = 0; i <= len; i++) {
        if (input[i] == delimiter || input[i] == '\0') {
            if (in_token && token_count < max_tokens) {
                int token_len = i - start;
                
                // Allocate space for token + null terminator
                char* token = kmalloc(token_len + 1);
                
                // Copy substring
                memcpy(token, input + start, token_len);
                token[token_len] = '\0';
                
                tokens[token_count++] = token;
                in_token = 0;
            }
            start = i + 1;
        } else {
            in_token = 1;
        }
    }

    // Terminate array with NULL
    if (token_count < max_tokens) {
        tokens[token_count] = NULL;
    }
    return tokens;

}