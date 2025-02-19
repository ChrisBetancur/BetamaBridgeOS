#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/list.h>
#include <string.h>

#include <stddef.h>  // For size_t
#include <kernel/memory.h>

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();

    puts("Hello, kernel World!\n");
    memory_init();

    puts("Memory initialized\n");

    void* ptr = kmalloc(100);
    puthex(ptr);
    puts("\n");
    void* ptr2 = kmalloc(2000);
    puthex(ptr2);
    puts("\n");
    void* ptr3 = kmalloc(50000);
    puthex(ptr3);
    puts("\n");  
}