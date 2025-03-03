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

    /*void* ptr  = kmalloc(4);
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

    kfree(ptr2);
    kfree(ptr3);

    print_allocated_heap();*/

	node_t* head = kmalloc(sizeof(struct node));
	head->data = 99;
	head->next = NULL;

	for (int i = 0; i < 10; i++) {
		append_node(head, i);
	}

	node_t* curr = head;

	while (curr != NULL) {
		puts("<addr: 0x");
		puthex(curr);
		puts("| data: ");
		putdec(curr->data);
		if (curr->next != NULL) {
			puts("| next_addr: ");
			puthex(curr->next);
		}
		puts(">");
		puts("\n");

		curr = curr->next;
	}
	
    framebuffer_init();
	framebuffer_set_background(COLOR_WHITE);
	
	
	gpu_puts("Hello, kernel World!\n");

	poll_cli_input();


}
