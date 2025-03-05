#include <drivers/sd.h>
#include <drivers/spi.h>

uint8_t sd_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    SPI0->CS |= SPI_CS_TA; // Start transaction

    // Send command (6 bytes)
    spi_transfer(cmd | 0x40); // Start bit + command
    spi_transfer((arg >> 24) & 0xFF);
    spi_transfer((arg >> 16) & 0xFF);
    spi_transfer((arg >> 8) & 0xFF);
    spi_transfer(arg & 0xFF);
    spi_transfer(crc);

    // Wait for response
    uint8_t response;
    for (int i = 0; i < 8; i++) {
        response = spi_transfer(0xFF);
        if (!(response & 0x80)) break;
    }

    SPI0->CS &= ~SPI_CS_TA; // End transaction
    return response;
}

void sd_init() {
    spi_init();

    puts("SPI done...\n");
    // Send 80 dummy clocks to wake up SD card
    for (int i = 0; i < 10; i++) spi_transfer(0xFF);
    puts("check\n");
    // Initialize SD card
    while (sd_command(0x00, 0x00000000, 0x95) != 0x01); // CMD0 (reset)
    puts("check\n");
    while (sd_command(0x08, 0x000001AA, 0x87) != 0x01); // CMD8 (check voltage)
    puts("check\n");
    do {
        sd_command(0x55, 0x00000000, 0x65); // CMD55 (prefix for ACMD)
    } while (sd_command(0x41, 0x40000000, 0x77) != 0x00); // ACMD41 (init)
    puts("check\n");
}

void sd_write_block(uint32_t sector, uint8_t *data) {
    // Send CMD24 (write single block)
    sd_command(0x18, sector, 0xFF); // For SDHC cards, sector = block address

    // Send start token (0xFE) and data
    SPI0->CS |= SPI_CS_TA;
    spi_transfer(0xFE); // Start token
    for (int i = 0; i < 512; i++) {
        spi_transfer(data[i]);
    }
    spi_transfer(0xFF); // Dummy CRC
    spi_transfer(0xFF);

    // Wait for write completion (poll BUSY bit)
    while (spi_transfer(0xFF) != 0xFF) {
        //puts("waiting...\n");
    }
    SPI0->CS &= ~SPI_CS_TA;
}

void sd_read_block(uint32_t sector, uint8_t *buffer) {
    // Send CMD17 (READ_SINGLE_BLOCK)
    sd_command(0x11, sector, 0xFF); // CMD17 for SDHC cards

    // Wait for data token (0xFE)
    while (spi_transfer(0xFF) != 0xFE);

    // Read 512 bytes into the buffer
    for (int i = 0; i < 512; i++) {
        buffer[i] = spi_transfer(0xFF);
    }

    // Discard CRC (optional)
    spi_transfer(0xFF);
    spi_transfer(0xFF);
}