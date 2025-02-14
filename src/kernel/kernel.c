#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>


/*static inline void mmio_write(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t*)reg = data;
}

static inline uint32_t mmio_read(uint32_t reg)
{
    return *(volatile uint32_t*)reg;
}


// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

enum
{
    // The GPIO registers base address.
    GPIO_BASE = 0x3F200000, // for raspi2 & 3, 0x20200000 for raspi1

    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    // The base address for UART.
    UART0_BASE = 0x3F201000, // for raspi2 & 3, 0x20201000 for raspi1

    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
    MAILBOX      = (0x3F00B880),
};

void read_uart_registers() {
    uart_registers[0] = mmio_read(UART0_DR);      // UART Data Register
    uart_registers[1] = mmio_read(UART0_RSRECR);  // UART Receive Status/Error Clear Register
    uart_registers[2] = mmio_read(UART0_FR);      // UART Flag Register
    uart_registers[3] = mmio_read(UART0_ILPR);    // UART IrDA Low-Power Counter Register
    uart_registers[4] = mmio_read(UART0_IBRD);    // UART Integer Baud Rate Divisor
    uart_registers[5] = mmio_read(UART0_FBRD);    // UART Fractional Baud Rate Divisor
    uart_registers[6] = mmio_read(UART0_LCRH);    // UART Line Control Register
    uart_registers[7] = mmio_read(UART0_CR);      // UART Control Register
    uart_registers[8] = mmio_read(UART0_IFLS);    // UART Interrupt FIFO Level Select Register
    uart_registers[9] = mmio_read(UART0_IMSC);    // UART Interrupt Mask Set/Clear Register
    uart_registers[10] = mmio_read(UART0_RIS);    // UART Raw Interrupt Status Register
    uart_registers[11] = mmio_read(UART0_MIS);    // UART Masked Interrupt Status Register
    uart_registers[12] = mmio_read(UART0_ICR);    // UART Interrupt Clear Register
    uart_registers[13] = mmio_read(UART0_DMACR);  // UART DMA Control Register
    uart_registers[14] = mmio_read(UART0_ITCR);   // UART Test Control Register
    uart_registers[15] = mmio_read(UART0_ITIP);   // UART Integration Test Input Register
    uart_registers[16] = mmio_read(UART0_ITOP);   // UART Integration Test Output Register
    uart_registers[17] = mmio_read(UART0_TDR);    // UART Test Data Register
}

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
}*/

/*void uart_putc(unsigned char c)
{
    while ( mmio_read(UART0_FR) & (1 << 5) ) { }
    mmio_write(UART0_DR, c);
}

void uart_puthex(uint32_t value)
{
    const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        uart_putc(hex_chars[(value >> (i * 4)) & 0xF]); // Extract and print hex digit
    }
}


unsigned char uart_getc()
{
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}*/

/*void enable_loopback() {
    uint32_t cr = mmio_read(UART0_CR);
    cr |= (1 << 7);
    mmio_write(UART0_CR, cr);
}*/

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{

    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();


    /*uint32_t timeout = 1000000;  // Adjust as needed
    while ((mmio_read(UART0_FR) & (1 << 4)) && timeout--) {
        //uart_puts("Waiting for data...\r\n");
    }
    if (timeout == 0) {
        uart_puts("Error: Timeout waiting for data!\r\n");
        return;
    }

    

    uart_puts("UART0_FR after read: ");
    uart_puthex(mmio_read(UART0_FR));
    uart_puts("\r\n");
    uint32_t mailbox_val = mmio_read(UART0_DR);  // Read back the data

    // Print the result
    uart_puts("UART0_DR read value: ");
    
    uart_puthex(mailbox_val);
    uart_puts("\r\n");*/

    



    puts("Hello, kernel World!\r\n");

    while (1) {
        puts("Type something:\r\n");
        //uint32_t mailbox_val = uart_getc();

        putc(getc());
        putc('\n');
    }
}