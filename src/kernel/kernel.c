#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/list.h>
#include <string.h>

#include <stddef.h>  // For size_t

void *k_memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

static inline uint32_t read_cpuid() {
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    return val;
}

int is_qemu() {
    uint32_t cpuid = read_cpuid();
    return (cpuid == 0x0B76);  // Example CPUID for QEMU ARM
}

typedef struct NODE_STRUCT node_t;

struct NODE_STRUCT {
    int data;
    PAIR_LIST_NODE(node_t);
} __attribute__((aligned(8)));

DEFINE_LIST_STRUCTURE(node_t);
DEFINE_LIST_FUNCTIONS(node_t);

#define MAX_NODES 10
static node_t all_nodes_array[MAX_NODES];
static node_t_list free_nodes;


void init_nodes() {
    puts("Initializing nodes...\r\n");

    // Initialize the list using the macro
    INIT_LIST(free_nodes);

    // Append all nodes to the free list
    for (int i = 0; i < MAX_NODES; i++) {
        puts("Appending node ");
        puthex(i);
        puts("\r\n");

        // Initialize the node
        all_nodes_array[i].data = i;
        all_nodes_array[i].next = NULL;
        all_nodes_array[i].prev = NULL;

        // Append the node to the free list
        append_node_t_list_node(&free_nodes, &all_nodes_array[i]);
    }

    puts("Nodes initialized.\r\n");
}


void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();

    puts("Entering kernel_main...\r\n");
    init_nodes();
    puts("Hello, kernel World!\r\n");

    while (1) {
        puts("Type something:\r\n");
        putc(getc());
        putc('\n');
    }
}