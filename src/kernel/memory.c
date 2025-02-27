#include <kernel/memory.h>
#include <common/stdio.h>

extern uint32_t __end;
extern uint32_t __start;

void zero_memory(uint32_t start, uint32_t num_bytes) {
    uint32_t *ptr32 = (uint32_t *) start; // 32-bit pointer
    uint32_t words = num_bytes / 4;
    uint8_t *ptr8;

    while (words--) {
        *ptr32++ = 0;
    }

    // Zero remaining bytes (if num_bytes isn't a multiple of 4)
    ptr8 = (uint8_t *) ptr32;
    num_bytes %= 4;
    while (num_bytes--) {
        *ptr8++ = 0;
    }
}


#define MIN_BATCH_SIZE 1024
#define MAX_BATCH_SIZE 65536

uint32_t calculate_batch_size(uint32_t total_free_pages) {
    uint32_t batch_size = total_free_pages / 100; // 1% of total free pages
    if (batch_size < MIN_BATCH_SIZE) batch_size = MIN_BATCH_SIZE;
    if (batch_size > MAX_BATCH_SIZE) batch_size = MAX_BATCH_SIZE;
    return batch_size;
}

// Functions won't be public, so no need to define them in the header file
// Page functions should be for internal use only (memory.c file)
DEFINE_LIST_STRUCTURE(page_t); // Will create list struct called page_t_list
DEFINE_LIST_FUNCTIONS(page_t);

static page_t_list free_pages;

void memory_init() {
    // The number of pages used by the kernel binary are for the bss, data, rodata, and text sections defined in the linker.ld file
    uint32_t num_pages_used = ((uint32_t) &__end) / PAGE_SIZE; // __end is the end of the kernel binary
    pages = (page_t*) &__end; // BUG FIX?

    int i;

    for (i = 0; i < num_pages_used; i++) {
        pages[i].vaddr_mapped = i * PAGE_SIZE;
        pages[i].paddr_base = i * PAGE_SIZE;
        pages[i].flags.present = 1;
        pages[i].flags.writable = 0;
        pages[i].flags.executable = 0;
    }

    //zero_memory(num_pages_used * PAGE_SIZE, memory_size - num_pages_used * PAGE_SIZE);
    INIT_LIST(free_pages);  

    num_pages = num_pages_used;

    heap_size = 0;
}

// When we allocate a page, we create the metadata for the page
void* alloc_page() {
    page_t* page = pop_page_t_list(&free_pages);
 
    if (page == NULL) {
        // Initialize a new batch of free pages
        uint32_t batch_size = 2; // Adjust based on performance testing
        uint32_t start_index = num_pages; // Calculate the start index of the next batch
        uint32_t end_index = start_index + batch_size;

        uint32_t memory_size = 0;



        //if (end_index > TOTAL_NUM_PAGES) end_index = TOTAL_NUM_PAGES;

        for (uint32_t i = start_index; i < end_index; i++) {
            pages[i].vaddr_mapped = i * PAGE_SIZE + TOTAL_NUM_PAGES * sizeof(page_t) + __end; // allocate page after page metadata array
            pages[i].paddr_base = (i * PAGE_SIZE) + TOTAL_NUM_PAGES * sizeof(page_t) + __end;
            pages[i].flags.present = 0;
            pages[i].flags.writable = 0;
            pages[i].flags.executable = 0;
            append_page_t_list(&free_pages, &pages[i]);
            num_pages++;
        }

        page = pop_page_t_list(&free_pages);
        page->flags.present = 1;
    }
    
    return page->paddr_base;
}

void split_block(heap_block_t* best_block, uint32_t best_diff, uint32_t size) {
    if (best_diff <= sizeof(heap_block_t) + 16) return;

    heap_block_t* new_block = (heap_block_t*)((uint32_t)best_block + size);
    new_block->size = best_block->size - size; 
    new_block->is_allocated = 0;

    new_block->next = best_block->next;
    if (new_block->next) new_block->next->prev = new_block;
    new_block->prev = best_block;
    best_block->next = new_block;

    best_block->size = size;
    best_block->is_allocated = 1;
}

heap_block_t* find_best_fit(uint32_t size) {
    heap_block_t* curr = heap_head;

    if (curr == NULL) return NULL;

    size += sizeof(heap_block_t);
    size = ALIGN(size, 8);

    int curr_diff, best_diff = 0x7fffffff;
    
    heap_block_t* best_block = NULL;

    while (curr && heap_size > 1) {
        curr_diff = curr->size - size;

        if (curr->size >= size && !curr->is_allocated) {

            best_diff = curr_diff;
            best_block = curr;
        }

        curr = curr->next;
    }

    if (best_diff > sizeof(heap_block_t) + 16)
        split_block(best_block, best_diff, size);

    return best_block;
}

void print_allocated_heap() {
    heap_block_t *current = heap_head;
    puts("Heap allocation map:\n");
    if (current == NULL) {
        puts("  Heap is empty.\n");
        return;
    }
    
    uint32_t block_num = 0;
    while (current != NULL) {
        puts("  Block ");
        putdec(block_num++);
        puts(" - Address: 0x");
        puthex((uint32_t) current);
        puts(" | Pages Used: ");
        putdec(num_pages);
        puts(" | Header Size: ");
        putdec(sizeof(heap_block_t));
        puts(" | Content Size: ");
        putdec(current->content_size);
        puts(" | Size: ");
        putdec(current->size);
        puts(" bytes | Status: ");
        if (current->is_allocated) {
            puts("ALLOCATED\n");
        } else {
            puts("FREE\n");
        }
        current = current->next;
    }
}


void* kmalloc(uint32_t size) {
    if (heap_size + size > KERNEL_HEAP_SIZE) {
        // MUST HANDLE OUT OF MEMORY ERROR
        puts("[KERNEL] Out of memory\n");
        return NULL;
    }

    heap_block_t* best_fit = find_best_fit(size);

    if (best_fit == NULL) {
        uint32_t num_pages_needed = (size + sizeof(heap_block_t) + PAGE_SIZE - 1) / PAGE_SIZE;
        void* pages_alloc[num_pages_needed];

        // MUST IMPLEMENT SAFEGAURD TO PREVENT ALLOCAING MORE THAN THE HEAP MEMORY LIMITS
        for (uint32_t i = 0; i < num_pages_needed; i++) {
            pages_alloc[i] = alloc_page();
            if (pages_alloc[i] == NULL) {
                // MUST HANDLE OUT OF MEMORY ERROR
                return NULL;
            }
        }
        best_fit = (heap_block_t*) pages_alloc[0];
        best_fit->size = num_pages_needed * PAGE_SIZE + sizeof(heap_block_t);

        best_fit->size = ALIGN(best_fit->size, 8);

        uint32_t best_diff = best_fit->size - size;
        if (best_diff > sizeof(heap_block_t) + 16)
            split_block(best_fit, best_diff, size + sizeof(heap_block_t));
    }

    if (heap_head == NULL) {
        heap_head = best_fit;
        heap_tail = best_fit;
        best_fit->is_allocated = 1;
        best_fit->content_size = size;
        heap_size++;
    }
    else {
        heap_tail->next = best_fit;
        best_fit->is_allocated = 1;
        best_fit->prev = heap_tail;
        heap_tail = best_fit;
        best_fit->content_size = size;
        heap_size++;

    } 
    
    heap_size += best_fit->size;

    return (void*)best_fit;
}

void kfree(void* ptr) {
    heap_block_t* block = (heap_block_t*) ptr;
    block->is_allocated = 0;

    while (block->prev != NULL && !block->prev->is_allocated) {
        block->prev->next = block->next;
        block->prev->size += block->size;
        block = block->prev;
    }

    while (block->next != NULL && !block->next->is_allocated) {
        block->size += block->next->size;
        block->next = block->next->next;
    }
}
