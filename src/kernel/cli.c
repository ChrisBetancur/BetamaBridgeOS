#include <kernel/cli.h>
#include <common/stdio.h>

void poll_cli_input() {
    int ch;
    do {
        ch = uart_read_input();
        if (ch != -1) {
            gpu_putc((unsigned char)ch);
            puts((unsigned char) ch);
        }
    } while (1); // Keep polling until data arrives
}