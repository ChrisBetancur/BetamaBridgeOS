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

char* hex_to_string(const char* hex_str) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0) {
        // Hex string should have an even number of characters
        return NULL;
    }

    size_t char_len = len / 2;
    char* result = (char*)kmalloc(char_len + 1); // +1 for the null terminator
    if (result == NULL) {
        // Memory allocation failed
        return NULL;
    }

    for (size_t i = 0, j = 0; i < len; i += 2, j++) {
        unsigned char high = hex_str[i];
        unsigned char low = hex_str[i + 1];

        // Convert high nibble
        if (high >= '0' && high <= '9') {
            high -= '0';
        } else if (high >= 'a' && high <= 'f') {
            high -= 'a' - 10;
        } else if (high >= 'A' && high <= 'F') {
            high -= 'A' - 10;
        } else {
            // Invalid character
            kfree(result);
            return NULL;
        }

        // Convert low nibble
        if (low >= '0' && low <= '9') {
            low -= '0';
        } else if (low >= 'a' && low <= 'f') {
            low -= 'a' - 10;
        } else if (low >= 'A' && low <= 'F') {
            low -= 'A' - 10;
        } else {
            // Invalid character
            kfree(result);
            return NULL;
        }

        result[j] = (high << 4) | low;
    }
    result[char_len] = '\0'; // Null-terminate the string

    return result;
}

void pointer_to_hex_str(void* ptr, char* buf, size_t buf_size) {
    // Check that the buffer is big enough
    if (buf_size < 11) {
        return;
    }
    
    // Cast pointer to an unsigned integer type
    uintptr_t addr = (uintptr_t)ptr;
    const char hex_digits[] = "0123456789ABCDEF";
    
    // Write the "0x" prefix
    buf[0] = '0';
    buf[1] = 'x';
    
    // Process 8 hex digits for 32-bit address.
    // For 64-bit, you would need 16 digits and adjust accordingly.
    for (int i = 0; i < 8; i++) {
        // Shift to get the corresponding nibble (4 bits)
        int shift = (7 - i) * 4;
        int nibble = (addr >> shift) & 0xF;
        buf[i + 2] = hex_digits[nibble];
    }
    
    buf[10] = '\0'; // Null-terminate the string
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
        if ((input[i] == delimiter || input[i] == '\0') && token_count < max_tokens) {
            int token_len = i - start;
                
                // Allocate space for token + null terminator
            char* token = kmalloc(token_len + 1);
                
            // Copy substring
            memcpy(token, input + start, token_len);
            token[token_len] = '\0';
                
            tokens[token_count++] = token;
            in_token = 0;
            //}
            start = i + 1;

            if(input[i] == '\0') {
                break;
            }

            if (token_count == max_tokens - 1) {
                int remaining_len = len - start;
                char* last_token = kmalloc(remaining_len + 1);
                memcpy(last_token, input + start, remaining_len);
                last_token[remaining_len] = '\0';
                tokens[token_count++] = last_token;
                break;
            }

        }
    }

    // Terminate array with NULL
    if (token_count < max_tokens) {
        tokens[token_count] = NULL;
    }
    return tokens;

}