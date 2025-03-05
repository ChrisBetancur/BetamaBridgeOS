#include <drivers/spi.h>
#include <kernel/uart.h>

void spi_init() {
    // Configure GPIO7-11 for SPI0 (ALT0)
    mmio_write(GPIO_BASE + 0x00, 0x240); // GPIO7-9 (ALT0)
    mmio_write(GPIO_BASE + 0x04, (0b100 << 0) | (0b100 << 3)); // GPIO10-11 (ALT0)

    // Enable pull-ups
    mmio_write(GPPUD, 0b10);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
    delay(150);
    mmio_write(GPPUDCLK0, 0);

    // Initialize SPI
    SPI0->CS = 0;
    SPI0->CLK = 250; // 1 MHz
    SPI0->CS = SPI_CS_CLEAR; // Clear FIFOs
    SPI0->CS &= ~(1 << 1); // CPHA=0
    SPI0->CS &= ~(1 << 0); // CPOL=0
}

uint8_t spi_transfer(uint8_t data) {
    // Wait until TX FIFO is not full
    while (SPI0->CS & SPI_CS_TXF) {}

    // Write data to TX FIFO
    SPI0->FIFO = data;

    // Wait until transfer is done (DONE bit set)
    while (!(SPI0->CS & SPI_CS_DONE)) {
        puts("waiting transfer...\n");
    }

    // Clear DONE bit by writing 1 to it
    SPI0->CS |= SPI_CS_DONE;

    // Read received data from RX FIFO
    return SPI0->FIFO;
}