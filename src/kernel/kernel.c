#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/list.h>
#include <string.h>
#include <stddef.h>
#include <kernel/memory.h>
#include <kernel/gpu.h>
#include <kernel/cli.h>
#include <drivers/sd.h>
#include <fs/fs.h>

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();

    memory_init();

	setup_fs();
	mount_sim_fs();
	
    framebuffer_init();
	framebuffer_set_background(COLOR_WHITE);

	
    shell_install();
	poll_cli_input();
}
