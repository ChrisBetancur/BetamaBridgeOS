#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#ifdef SIM_FS

#define FS_SIZE 4 * 1024 * 1024

#endif

#define BLOCK_SIZE 1024

#define MAX_BLOCKS FS_SIZE / BLOCK_SIZE

#define CEILING(x, y) ((x) + (y) - 1) / (y)

// Max 512 files, more than enough for the demo file system
#define MAX_INODES 512
// Max amount of bytes needed for inode bitmap cieling(512/8)=64 bytes
#define INODE_BITMAP_SIZE CEILING(MAX_INODES, 8)

// Each zone is 2 blocks therefore 2048 blocks
#define MAX_ZONES MAX_BLOCKS / 2

// Max amount of bytes needed for zone bitmap cieling((2048+8−1)/8)=256 bytes
#define ZONE_BITMAP_SIZE CEILING(MAX_ZONES, 8)

// 0,1, 2 .., nth index
static inline void set_bit(uint8_t* bitmap, uint32_t n) {
    bitmap[n/8] |= (1 << (n%8));
}

// 0,1, 2 .., nth index
static inline void clear_bit(uint8_t* bitmap, uint32_t n) {
    bitmap[n/8] &= ~(1 << (n%8));
}

// 0,1, 2 .., nth index
static inline uint32_t is_bit_set(uint8_t* bitmap, uint32_t n) {
    return (bitmap[n/8] & (1 << (n%8))) != 0;
}


typedef struct superblock {
    uint32_t fs_size; 
    uint32_t num_inodes;

    uint32_t num_inode_bitmap_blocks;
    uint32_t num_zone_bitmap_blocks;

    uint8_t num_zone_per_block;

    uint32_t block_size;

    // Block Address=Zone Number*2^shift_count=Zone Number≪shift_count.
    // Zone Number=Block Address/2^shift_count=Block Address≫shift_count.
    uint32_t shift_count;

    // Bitmaps will be stored in super block since they are very small
    // Normally they would be stored in seperate blocks
    uint8_t inode_bitmap[INODE_BITMAP_SIZE]; // 64 bytes
    // Zones make up the entire disk including superblock and bootblock,
    // so offset is need to access the zones that contain the actual files
    uint8_t zone_bitmap[ZONE_BITMAP_SIZE]; // 256 bytes

    uint32_t first_free_inode;
    uint32_t first_free_zone;

    // Start of the data zones for the files. Will use this as the bit-index of what zone of the bitmap starts the file data
    // (Used as offset for bit setting)
    uint32_t first_data_zone;
    // Used as bit-index for bitmap
    uint32_t root_inode;

    // Major device number (e.g., 8)
    uint16_t major_device;
    // Minor device number (e.g., 1)
    uint16_t minor_device;

    // bit map to define flags
    uint32_t flags;

    uint32_t magic;
    uint32_t version;

    uint8_t padding[647]; // Current total is 377 bytes

} __attribute__((packed)) superblock_t; // Total should be 1024 bytes

#define I_REGULAR 0x8000    // Regular file
#define I_DIRECTORY 0x4000  // Directory
#define I_READ  0x0100      // Read permission
#define I_WRITE 0x0080      // Write permission
#define I_EXEC  0x0040      // Execute permission
// inode.mode = I_REGULAR | I_READ | I_WRITE;

typedef struct inode {
    // Define constants for the mode
    // Mode is the type, flags, permsisions
    uint16_t mode;
    uint32_t zones[3]; // 3*(2*1024)=6kb size for data for each file
    uint32_t num_links;

    uint32_t file_size;
} inode_t;

char* get_inode_name(uint32_t inode_num);

#define FILENAME_SIZE 21

typedef struct dir_entry {
    uint32_t inode_link; // inode number that the entry points to
    char filename[FILENAME_SIZE];
} dir_entry_t;

extern uint32_t current_dir_id;
static uint32_t current_num_inode; // MAX IS 512

void setup_fs();

void mount_sim_fs();

void create_file(const char* filename, const char* data, size_t data_size);

void create_dir(const char* dirname);

char** list_dir();

void change_dir(const char* dirname);

void write_file(const char* filename, const char* data, size_t data_size);

void* read_file(const char* filename);

#ifdef SIM_FS

// AFTER HEAP
#define FS_START_ADDR 0x540000

void demo_write();

#endif

#endif