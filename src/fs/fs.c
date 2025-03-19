#include <fs/fs.h>
#include <memory.h>
#include <common/stdio.h>

#ifdef SIM_FS

// data must be of length BLOCK_SIZE, if smaller, pad it with 0s
static void sim_write_block(void* data, uint32_t block_num) {
    uint32_t write_addr = FS_START_ADDR + (block_num * BLOCK_SIZE);
    printf("Writing block %d to addr=%x\n", block_num, write_addr);

    void* sim_disk = write_addr;

    zero_memory(write_addr, BLOCK_SIZE);

    memcpy(sim_disk, data, BLOCK_SIZE);
}

static void* sim_read_block(uint32_t block_num) {
    uint32_t read_addr = FS_START_ADDR + (block_num * BLOCK_SIZE);
    printf("Reading block %d from addr=%x\n", block_num, read_addr);

    return (void*) read_addr;
}

#endif

static void log_inode(superblock_t* sb, inode_t* inode) {
    printf("Inode %x:\n", *inode);
    printf("  Mode: 0x%x\n", inode->mode);
    printf("  File Size: %d\n", inode->file_size);
    printf("  Num Links: %d\n", inode->num_links);
    printf("  Zones: { 0x%x, 0x%x, 0x%x }\n",
        inode->zones[0],
        inode->zones[1],
        inode->zones[2]);
                   
    // For each non-zero zone in the inode, print its contents.
    for (int j = 0; j < 3; j++) {
        uint32_t zone = inode->zones[j];
        if (zone == 0) continue;
                
        // Convert zone to a block number.
        uint32_t block_num = zone << sb->shift_count;
        uint8_t *zone_data = (uint8_t *)sim_read_block(block_num);
        printf("  Zone %d (Block %d):\n", j, block_num);
                
        // If inode is a directory, print its directory entries.
        if (inode->mode & I_DIRECTORY) {
            dir_entry_t *entries = (dir_entry_t *)zone_data;
            int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);
            for (int k = 0; k < num_entries; k++) {
                if (entries[k].inode_link != 0) {
                    printf("    Entry %d: inode_link = %d, filename = %s\n",
                        k, entries[k].inode_link, entries[k].filename);
                }
            }
        }
        // If inode is a regular file, print a preview of its content.
        else if (inode->mode & I_REGULAR) {
            printf("    File Content (first 64 bytes): %s\n", 64, zone_data);
        }
    }
}



void print_byte_bits(uint8_t byte) {
    // Loop from bit 7 (most significant) to bit 0 (least significant)
    for (int i = 7; i >= 0; i--) {
        // If printf supports %c, we can use that to print a character.
        // Otherwise, you'll need to implement a lower-level function to output a character.
        printf("%c", (byte & (1 << i)) ? '1' : '0');
    }
}


static uint32_t allocate_zone(superblock_t* sp) {
    for (int i = 0; i < ZONE_BITMAP_SIZE; i++) {
        uint32_t status = is_bit_set(sp->zone_bitmap, i);

        if (status == 0) {
            set_bit(sp->zone_bitmap, i);
            return i;
        }
    }
    return -1;
}

void setup_fs() {
    // Block 0 is reserved for bootblock
        superblock_t sp =  {
        .fs_size = FS_SIZE,
        .num_inodes = MAX_INODES,
        .num_inode_bitmap_blocks = 0,
        .num_zone_bitmap_blocks = 0,
        .num_zone_per_block = 2,
        .block_size = BLOCK_SIZE,
        .shift_count = 1, // 2^1 block per zone
        .inode_bitmap = {0}, // 1 means used
        .zone_bitmap = {0},
        .first_free_inode = 0,
        .first_data_zone = 1 + 1 + (sizeof(struct inode) * MAX_INODES), // bootbloc + superblock +(num of blocks for inode metadata)
        .root_inode = 0,
        .major_device = 0,
        .minor_device = 0,
        .magic = 0xdeadbeaf,
        .version = 1,
    };

    set_bit(sp.zone_bitmap, 0); // blocks 1-2 = zone 1
    

    inode_t* metadata_table = kmalloc(MAX_INODES * sizeof(inode_t));

    for (int i = 0; i < MAX_INODES; i++) {
        metadata_table[i].mode = 0;
        memset(metadata_table[i].zones, 0, 3 * (BLOCK_SIZE * 2));
        metadata_table[i].num_links = 0;
        metadata_table[i].file_size = 0;
    }

    set_bit(sp.zone_bitmap, 1); // blocks 3-4 = zone 2

    sim_write_block(&sp, 1);
    sim_write_block(metadata_table, 2);

    //superblock_t* sp = (superblock_t*) sim_read_block(1);
    //inode_t* metadata_table = (inode_t*) sim_read_block(2);
}

static void allocate_inode (
        superblock_t* sp,
        inode_t* metadata_table, 
        uint16_t mode, 
        uint32_t inode_num, 
        uint32_t parent_inode_num, 
        void* data, 
        size_t data_size) {

    printf("Allocating node\n");
    set_bit(sp->inode_bitmap, inode_num);
    metadata_table[inode_num].mode = mode;
    metadata_table[inode_num].file_size = 0;

    uint8_t local_entries[BLOCK_SIZE];

    // 3 zones

    if (mode & I_DIRECTORY) {
        metadata_table[inode_num].zones[0] = allocate_zone(sp); // allocate only one zone, allocate more if more inodes are linekd to dire
        metadata_table[inode_num].num_links = 2;
        uint32_t block_num = metadata_table[inode_num].zones[0] << sp->shift_count;
        printf("Directory zone %d\n", metadata_table[inode_num].zones[0]);
        
        memcpy(local_entries, sim_read_block(block_num), BLOCK_SIZE);
        dir_entry_t* entries = (dir_entry_t*)local_entries;
        
        entries[0].inode_link = inode_num;
        memset(entries[0].filename, 0, FILENAME_SIZE);
        const char* name = ".";
        size_t name_len = strlen(name);
        memcpy(entries[0].filename, name, name_len);

        entries[1].inode_link = parent_inode_num; 
        memset(entries[1].filename, 0, FILENAME_SIZE);
        const char* parent_name = "..";
        size_t parent_name_len = strlen(parent_name);
        memcpy(entries[1].filename, parent_name, parent_name_len);

        sim_write_block(entries, block_num);
    }
    else if (mode & I_REGULAR) {
        printf("File writing->\n");
        metadata_table[inode_num].num_links = 1;

        //uint32_t num_zones = CEILING((BLOCK_SIZE * 2), data_size);

        uint32_t zone_size = BLOCK_SIZE * 2; // 2048 bytes (power of two)
        uint32_t quotient = data_size >> 11; // Divide by 2048 via bit shift (equivalent to data_size / zone_size)
        uint32_t remainder = data_size & (zone_size - 1); // Remainder via bitwise AND (equivalent to data_size % zone_size)
        uint32_t num_zones = quotient + (remainder != 0);

        uint32_t bytes_written = 0;

        for (int i = 0; i < num_zones; i++) {
            metadata_table[inode_num].zones[i] = allocate_zone(sp);
            printf("Regular zone %d\n", metadata_table[inode_num].zones[i]);
            uint32_t block_num = metadata_table[inode_num].zones[i] << sp->shift_count;

            uint32_t chunk_size = (data_size - bytes_written) > (BLOCK_SIZE * 2) 
                          ? (BLOCK_SIZE * 2) 
                          : data_size - bytes_written;


            uint8_t chunk[BLOCK_SIZE];
            memset(chunk, 0, BLOCK_SIZE);

            memcpy(chunk, data + bytes_written, chunk_size);
            sim_write_block(&chunk, block_num);

            bytes_written += chunk_size;
        }
    }

    sim_write_block(sp, 1);
    sim_write_block(metadata_table, 2);
}

void dump_fs(void) {
    // --- Read the superblock ---
    uint8_t sb_buf[BLOCK_SIZE];
    memcpy(sb_buf, sim_read_block(1), BLOCK_SIZE);
    superblock_t *sb = (superblock_t *)sb_buf;
    
    // --- Read the inode metadata table (assumed to be in block 2) ---
    uint8_t mt_buf[BLOCK_SIZE];
    memcpy(mt_buf, sim_read_block(2), BLOCK_SIZE);
    inode_t *inodes = (inode_t *)mt_buf;
    
    printf("\n=== Allocated Inode Table ===\n");
    for (int i = 0; i < MAX_INODES; i++) {
        // Only print inodes that are marked as allocated in the inode bitmap.
        if (is_bit_set(sb->inode_bitmap, i)) {
            printf("Inode %d:\n", i);
            printf("  Mode: 0x%x\n", inodes[i].mode);
            printf("  File Size: %d\n", inodes[i].file_size);
            printf("  Num Links: %d\n", inodes[i].num_links);
            printf("  Zones: { 0x%x, 0x%x, 0x%x }\n",
                   inodes[i].zones[0],
                   inodes[i].zones[1],
                   inodes[i].zones[2]);
                   
            // For each non-zero zone in the inode, print its contents.
            for (int j = 0; j < 3; j++) {
                uint32_t zone = inodes[i].zones[j];
                if (zone == 0) continue;
                
                // Convert zone to a block number.
                uint32_t block_num = zone << sb->shift_count;
                uint8_t *zone_data = (uint8_t *)sim_read_block(block_num);
                printf("  Zone %d (Block %d):\n", j, block_num);
                
                // If inode is a directory, print its directory entries.
                if (inodes[i].mode & I_DIRECTORY) {
                    dir_entry_t *entries = (dir_entry_t *)zone_data;
                    int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);
                    for (int k = 0; k < num_entries; k++) {
                        if (strcmp(entries[k].filename, "") != 0) {
                            printf("    Entry %d: inode_link = %d, filename = \"%s\"\n",
                                   k, entries[k].inode_link, entries[k].filename);
                        }
                    }
                }
                // If inode is a regular file, print a preview of its content.
                else if (inodes[i].mode & I_REGULAR) {
                    printf("    File Content (first 64 bytes): %s\n", 64, zone_data);
                }
            }
        }
    }
    
    // --- Dump allocated zones based solely on the zone bitmap ---
    printf("\n=== Allocated Zones ===\n");
    // MAX_ZONES is defined as MAX_BLOCKS / 2.
    for (int i = 0; i < MAX_ZONES; i++) {
        if (is_bit_set(sb->zone_bitmap, i)) {
            uint32_t block_num = i << sb->shift_count;
            uint8_t *zone_data = (uint8_t *)sim_read_block(block_num);

            printf("Zone %d (Block %d) allocated. First 64 bytes: %s\n",
                   i, block_num, zone_data);
        }
    }
}

void create_file(const char* filename, const char* data, size_t data_size) {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    current_num_inode++;
    
    uint16_t file_mode = I_REGULAR | I_READ | I_WRITE | I_EXEC;

    allocate_inode(sp, metadata_table, file_mode, current_num_inode, current_dir_id, data, strlen(data));

    // Update the directory entry
    metadata_table[current_dir_id].file_size += sizeof(dir_entry_t);
    metadata_table[current_dir_id].num_links += 1;
    uint32_t entry_num = metadata_table[current_dir_id].num_links;
    

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;
        
    entries[metadata_table[current_dir_id].num_links - 1].inode_link = current_num_inode;
    memset(entries[metadata_table[current_dir_id].num_links - 1].filename, 0, FILENAME_SIZE);
    size_t name_len = strlen(filename);
    memcpy(entries[metadata_table[current_dir_id].num_links - 1].filename, filename, name_len);

    sim_write_block(metadata_table, 2);
    sim_write_block(entries, block_num);
}

void create_dir(const char* dirname) {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    current_num_inode++;

    uint16_t dir_mode = I_DIRECTORY | I_READ | I_WRITE | I_EXEC;
    allocate_inode(sp, metadata_table, dir_mode, current_num_inode, current_dir_id, NULL, NULL);

    metadata_table[current_dir_id].file_size += sizeof(dir_entry_t);
    metadata_table[current_dir_id].num_links += 1;
    uint32_t entry_num = metadata_table[current_dir_id].num_links;
    

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;
        
    entries[metadata_table[current_dir_id].num_links - 1].inode_link = current_num_inode;
    memset(entries[metadata_table[current_dir_id].num_links - 1].filename, 0, FILENAME_SIZE);
    size_t name_len = strlen(dirname);
    memcpy(entries[metadata_table[current_dir_id].num_links - 1].filename, dirname, name_len);

    sim_write_block(metadata_table, 2);
    sim_write_block(entries, block_num);
}

char** list_dir() {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;

    int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);
    char** dir_list = kmalloc(num_entries * sizeof(char*));

    for (int k = 0; k < num_entries; k++) {
        if (strcmp(entries[k].filename, "") != 0) {
            dir_list[k] = entries[k].filename;
        }
    }

    return dir_list;
}

void change_dir(const char* dirname) {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;

    int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);
    for (int k = 0; k < num_entries; k++) {
        if (strcmp(entries[k].filename, dirname) == 0) {
            current_dir_id = entries[k].inode_link;
            break;
        }
    }
}

void write_file(const char* filename, const char* data, size_t data_size) {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;

    int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);
    for (int k = 0; k < num_entries; k++) {
        if (strcmp(entries[k].filename, filename) == 0) {
            uint32_t inode_num = entries[k].inode_link;

            //uint32_t num_zones = CEILING((BLOCK_SIZE * 2), data_size);

            uint32_t zone_size = BLOCK_SIZE * 2; // 2048 bytes (power of two)
            uint32_t quotient = data_size >> 11; // Divide by 2048 via bit shift (equivalent to data_size / zone_size)
            uint32_t remainder = data_size & (zone_size - 1); // Remainder via bitwise AND (equivalent to data_size % zone_size)
            uint32_t num_zones = quotient + (remainder != 0);

            uint32_t bytes_written = 0;

            for (int i = 0; i < num_zones; i++) {
                uint32_t block_num = metadata_table[inode_num].zones[i] << sp->shift_count;

                uint32_t chunk_size = (data_size - bytes_written) > (BLOCK_SIZE * 2) 
                            ? (BLOCK_SIZE * 2) 
                            : data_size - bytes_written;


                uint8_t chunk[BLOCK_SIZE];
                memset(chunk, 0, BLOCK_SIZE);

                memcpy(chunk, data + bytes_written, chunk_size);
                sim_write_block(&chunk, block_num);

                bytes_written += chunk_size;
            }
        }
    }
}

void* read_file(const char* filename) {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    uint32_t block_num = metadata_table[current_dir_id].zones[0] << sp->shift_count;
        
    uint32_t zone = metadata_table[current_dir_id].zones[0];// NEED A WAY TO CHECK IF ZONE IS FULL?

    uint8_t zone_data[BLOCK_SIZE];
    memcpy(zone_data, sim_read_block(block_num), BLOCK_SIZE);
    dir_entry_t *entries = (dir_entry_t *)zone_data;

    int num_entries = BLOCK_SIZE / sizeof(dir_entry_t);

    char* data = NULL;
    for (int k = 0; k < num_entries; k++) {
        if (strcmp(entries[k].filename, filename) == 0) {
            uint32_t inode_num = entries[k].inode_link;
            inode_t* inode = &metadata_table[inode_num];
            data = kmalloc(inode->file_size);
            data = "";

            // For each non-zero zone in the inode, attatch to returned output.
            for (int j = 0; j < 3; j++) {
                uint32_t zone = inode->zones[j];
                if (zone == 0) continue;
                        
                // Convert zone to a block number.
                uint32_t block_num = zone << sp->shift_count;
                uint8_t *zone_data = (uint8_t *)sim_read_block(block_num);
                strcat(data, zone_data);
            }
            break;
        }
    }

    return (void*) data;
}


void mount_sim_fs() {
    uint8_t local_sp[BLOCK_SIZE];
    uint8_t local_metadata_table[BLOCK_SIZE];

    memcpy(local_sp, sim_read_block(1), BLOCK_SIZE);
    superblock_t* sp = (superblock_t*)local_sp;

    memcpy(local_metadata_table, sim_read_block(2), BLOCK_SIZE);
    inode_t* metadata_table = (inode_t*) local_metadata_table;

    uint16_t mode = I_DIRECTORY | I_EXEC;

    current_num_inode = 0;

    // Allocate root directory inode
    allocate_inode(sp, metadata_table, mode, 0, 0, NULL, NULL);
    current_dir_id = 0;

    create_file("test.txt", "Hello File World!", 17);
    create_file("test2.txt", "Hello File World 2!", 19);

    create_dir("test_dir");
    change_dir("test_dir");
    
    create_file("test.txt", "Hello File World inside another dir!", 17);
    write_file("test.txt", "Hello File World inside another dir! Updated", 50);

    char** dir_list = list_dir();
    char* data = read_file("test.txt");
    
    
    dump_fs();

    //printf("\nData: %s\n", data);
}