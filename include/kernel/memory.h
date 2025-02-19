#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <common/list.h>

// If run on hardware, atags will be passed and used
// Need to research and understand how to define the atags to set up the memory correctly
#ifndef QEMU 
// FUTURE ATAGS DEFINITIONS
#endif

#define ALIGN(addr, alignment) (((addr) + ((alignment) - 1)) & ~((alignment) - 1))

uint32_t get_kernel_memory_size_qemu();

typedef struct page page_t;


typedef struct {
    uint32_t present    : 1;  // 1 bit - Page is present in memory
    uint32_t writable   : 1;  // 1 bit - Page is writable
    uint32_t executable : 1;  // 1 bit - Page is executable
    uint32_t reserved   : 29; // 29 bits reserved for future use
} page_flags_t;

// Metadata for the page
struct page {
    uint32_t vaddr_mapped;
    uint32_t paddr_base;
    page_flags_t flags;
    
    PAIR_LIST_NODE(page_t);
};

#define PAGE_SIZE 4096 // 4KB

static uint32_t num_pages; // Current number of pages being used

static page_t* pages; // Array of all metadatapages (free and allocated)

void memory_init();

void zero_memory(uint32_t start, uint32_t num_bytes);

void* alloc_page();

#define KERNEL_HEAP_START ((void*)(&__end + PAGE_SIZE))
#define KERNEL_HEAP_SIZE (4 * 1024 * 1024) // 4mb

// struct serves as the metadata for the heap, after the metadata is ini memory, the data of the block is inserted after the metadata
typedef struct heap_block {
    struct heap_block* next;
    struct heap_block* prev;
    uint32_t size;
    uint32_t is_allocated;
} heap_block_t;

static heap_block_t* heap_head;
static heap_block_t* heap_tail;
static uint32_t heap_size;

void* kmalloc(uint32_t size);

#endif