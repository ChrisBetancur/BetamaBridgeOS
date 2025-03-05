#include <kernel/cli.h>
#include <common/stdio.h>

void poll_cli_input() {
    gpu_puts(CLI_PROMPT);
    int ch;
    do {
        ch = uart_read_input();
        if (ch != -1) {
            gpu_putc((unsigned char)ch);
        }
        if (ch == '\n' || ch == '\r') {
            gpu_puts(CLI_PROMPT);
        }
    } while (1);
}