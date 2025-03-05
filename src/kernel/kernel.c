#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/list.h>
#include <string.h>
#include <stddef.h>
#include <kernel/memory.h>
#include <kernel/gpu.h>
#include <kernel/cli.h>
#include <drivers/sd.h>

typedef struct node {
	int data;
	struct node* next;
} node_t;

void append_node(node_t* head, int data) {
	node_t* curr = head;


	while (curr->next != NULL) {
		curr = curr->next;
	}
	curr->next = kmalloc(sizeof(struct node));
	curr->next->data = data;
	putdec(curr->next->data);
	puts("\n");
	curr->next->next = NULL;
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();

    puts("Hello, kernel World!\n");
    memory_init();

    puts("Memory initialized\n");

	sd_init();
	puts("SD initialized\n");

	// Test write and read
	uint8_t write_buffer[512];
	uint8_t read_buffer[512];
	
	// Fill write_buffer with test data (e.g., 0xAA)
	memset(write_buffer, 0xAA, 512);

	// Write to sector 0x1000
	sd_write_block(0x1000, write_buffer);

	// Read back from sector 0x1000
	/*sd_read_block(0x1000, read_buffer);

	// Compare buffers
	if (memcmp(write_buffer, read_buffer, 512) == 0) {
		puts("Write verified successfully!\n");
	} else {
		puts("Write verification failed!\n");
	}*/
	
    framebuffer_init();
	framebuffer_set_background(COLOR_WHITE);

	

	poll_cli_input();
}
