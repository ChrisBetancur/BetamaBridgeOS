#ifndef SPI_H
#define SPI_H

#include <stdint.h>

// Set up SPI to communicate to SD cards
#define SPI0_BASE 0x3F204000

typedef struct {
    volatile uint32_t CS;    // Control/Status
    volatile uint32_t FIFO;  // Data FIFO
    volatile uint32_t CLK;   // Clock Divider
    // ... other registers (not needed for basic writes)
} spi_registers_t;

#define SPI0 ((spi_registers_t*)SPI0_BASE)

// SPI Control/Status (CS) Register Bits
#define SPI_CS_TA    (1 << 7)  // Transfer Active
#define SPI_CS_CLEAR (3 << 4)  // FIFO Clear
#define SPI_CS_DONE  (1 << 4)  // Transfer Done
#define SPI_CS_TXF   (1 << 18) // TX FIFO Full
#define SPI_CS_RXD   (1 << 17) // RX FIFO has data


void spi_init();

uint8_t spi_transfer(uint8_t data);

#endif