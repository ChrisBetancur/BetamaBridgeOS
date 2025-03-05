#ifndef SD_H
#define SD_H

#include <stdint.h>

uint8_t sd_command(uint8_t cmd, uint32_t arg, uint8_t crc);

void sd_init();

void sd_write_block(uint32_t sector, uint8_t *data);

void sd_read_block(uint32_t sector, uint8_t *buffer);

#endif