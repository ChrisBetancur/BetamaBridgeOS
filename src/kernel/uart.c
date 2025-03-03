#include <kernel/uart.h>

void uart_init()
{

    mmio_write(UART0_CR, 0x00000000);

    mmio_write(GPPUD, 0x00000000);
    delay(150);

    // Enable UART transmit and receive
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPPUDCLK0, 0x00000000);

    mmio_write(UART0_ICR, 0x7FF);

    mmio_write(UART0_IBRD, 1);
    mmio_write(UART0_FBRD, 40);

    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void enable_loopback() {
    uint32_t cr = mmio_read(UART0_CR);
    cr |= (1 << 7);
    mmio_write(UART0_CR, cr);
}

int uart_read_input() {
    if (mmio_read(UART0_FR) & (1 << 4)) { // Check RXFE bit (FIFO empty)
        return -1;
    }
    return (char)mmio_read(UART0_DR);
}