#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/list.h>
#include <string.h>
#include <stddef.h>
#include <kernel/memory.h>

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();

    puts("Hello, kernel World!\n");
    memory_init();

    puts("Memory initialized\n");

    void* ptr  = kmalloc(4);
    puthex(ptr);
    puts("\n");
    void* ptr2 = kmalloc(4);
    puthex(ptr2);
    puts("\n");
    void* ptr3 = kmalloc(4);
    puthex(ptr3);
    puts("\n");  
    void* ptr4 = kmalloc(4);
    puthex(ptr4);
    puts("\n");
    void* ptr5 = kmalloc(4);
    puthex(ptr5);
    puts("\n");

    print_allocated_heap();

}