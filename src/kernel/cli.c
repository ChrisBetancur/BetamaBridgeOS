#include <kernel/cli.h>
#include <common/stdio.h>

uint32_t cmd_index = 0;

cmd_entry_t cmd_entries[CMD_ENTRIES];

// MAY REPLACE THIS WITH A LEXER TO BETTER READ TERMINAL INPUT
// CREATE A TUPLE STRUCTURE

static void echo(void* data) {
    gpu_puts(data);
    gpu_puts("\n");
}


static cmd_entry_t* get_cmd_entry(char cmd_str[]) {
    for (int i = 0; i < CMD_ENTRIES; i++) {
        if (strcmp(cmd_entries[i].name, cmd_str) == 0) {
            return &cmd_entries[i];
        }
    }
    return NULL;
}

void register_cmd(const char* name, cmd_t handler) {
    if (cmd_index >= CMD_ENTRIES) return;
    
    cmd_entries[cmd_index].name = name;
    cmd_entries[cmd_index].handler = handler;
    cmd_index++;
}



void handle_cmd(char buffer[]) {
    //gpu_puts(buffer);
    // WORK ON SPLITTING STRING
    char **tuple = split_str(buffer, ' ', 2);
    if (!tuple || !tuple[0]) return;

    cmd_entry_t *entry = get_cmd_entry(tuple[0]);
    if (!entry || !entry->handler) {
        gpu_puts("Command not found\r\n");
        return;
    }

    // Pass remaining arguments (if any)
    char* args = tuple[1] ? tuple[1] : "";
    entry->handler(args);
}

void shell_install() {
    register_cmd("echo", echo);  // Proper registration with both name and handler
}

void poll_cli_input() {
    gpu_puts(CLI_PROMPT);
    int ch;
    char buffer[MAX_CMD_SIZE];
    int buf_index = 0;
    do {
        ch = uart_read_input();
        
        if (ch != -1 && buf_index < MAX_CMD_SIZE - 1) {
            buffer[buf_index++] = (char)ch;
            buffer[buf_index] = '\0';
            gpu_putc((char)ch); 
        
        }
        if (ch == '\n' || ch == '\r') {
            handle_cmd(buffer);
            gpu_puts(CLI_PROMPT);
            buf_index = 0;
            //gpu_puts(buffer);
        }

        
    } while (1);
}