#include <kernel/cli.h>
#include <common/stdio.h>
#include <fs/fs.h>
#include <kernel/framebuffer.h>
#include <kernel/gpu_utils.h>

uint32_t cmd_index = 0;
uint32_t current_dir_id; // from fs.h

cmd_entry_t cmd_entries[CMD_ENTRIES];

// MAY REPLACE THIS WITH A LEXER TO BETTER READ TERMINAL INPUT
// CREATE A TUPLE STRUCTURE

static void echo(void* data) {
    gpu_puts(data);
    gpu_puts("\n");
}

static void mkdir(void* data) {
    create_dir(data);
}

// TOUCH WILL TEMPORAILTY CREATE FILE WITH PREDETERMINED DATA INSIDE
static void touch(void* data) {
    create_file(data, "", 0);
}

static void ls(void* data) {
    char** dir_list = list_dir();
    char** dir_list_addr = list_dir_addr();

    int i;
    for (i = 0; dir_list[i] != NULL || dir_list[i] == ""; i++) { // || "" is a temp fix, don't know where the bug is coming from
        gpu_puts(dir_list_addr[i]);
        gpu_puts(" ");
        gpu_puts(dir_list[i]);
        gpu_puts("\n");
    }
    char ch_i[2];
    int_to_ascii(i, ch_i);
    gpu_puts("Total Items: ");
    gpu_puts(ch_i);
    gpu_puts("\n");
}

static void write(void* data) {
    char** tuple = split_str((char*)data, ' ', 2);

    char** dir_list = list_dir();

    int i;
    for (i = 0; dir_list[i] != NULL; i++) {
        if (strcmp(dir_list[i], tuple[0]) == 0) {
            break;
        }
    }
    
    if (dir_list[i] == NULL) {
        //gpu_puts("File not found\n");
        create_file(tuple[0], tuple[1], strlen(tuple[1]));
        gpu_puts("File written\n");
        gpu_puts("File name: ");
        gpu_puts(tuple[0]);
        gpu_puts("\n");
        gpu_puts("Data: ");
        gpu_puts(tuple[1]);
        gpu_puts("\n");
        gpu_puts("Data size: ");
        char ch_i[2];
        int_to_ascii(strlen(tuple[1]), ch_i);
        gpu_puts(ch_i);
        gpu_puts("\n");

        return;
    }

    /*write_file(dir_list[i], tuple[1], strlen(tuple[1]));

    gpu_puts("File written\n");
    gpu_puts("File name: ");
    gpu_puts(dir_list[i]);
    gpu_puts("\n");
    gpu_puts("Data: ");
    gpu_puts(tuple[1]);
    gpu_puts("\n");
    gpu_puts("Data size: ");
    char ch_i[2];
    int_to_ascii(strlen(tuple[1]), ch_i);
    gpu_puts(ch_i);
    gpu_puts("\n");*/

    gpu_puts("Currently Unable rewrite to a file...\n");
}

static void cd(void* data) {
    change_dir(data);
}

static void cat(void* data) {
    char* file_data = read_file(data);
    gpu_puts(file_data);
    gpu_puts("\n");
}

static void pwd(void* data) {
    char* dir_name = get_inode_name(current_dir_id);
    gpu_puts(dir_name);
    gpu_puts("\n");
}


static cmd_entry_t* get_cmd_entry(char cmd_str[]) {
    for (int i = 0; i < cmd_index; i++) {
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
    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
        buffer[--len] = '\0';
    }
    char **tuple = split_str(buffer, ' ', 2);

    if (!tuple || !tuple[0]) return;

    cmd_entry_t *entry = get_cmd_entry(tuple[0]);
    if (!entry || !entry->handler) {
        gpu_puts("\nCommand not found\n");
        return;
    }
    
    char* args = tuple[1] ? tuple[1] : "";
    entry->handler(args);
}

void shell_install() {
    register_cmd("echo", echo);  // Proper registration with both name and handler
    register_cmd("mkdir", mkdir);
    register_cmd("touch", touch);
    register_cmd("ls", ls);
    register_cmd("cd", cd);
    register_cmd("cat", cat);
    register_cmd("pwd", pwd);
    register_cmd("write", write);
}

void poll_cli_input() {

    char* current_dir_name = get_inode_name(current_dir_id);
    char dir_prompt = kmalloc(100);
    dir_prompt = "";
    dir_prompt = strcat(dir_prompt, CLI_PROMPT);
    dir_prompt = strcat(dir_prompt, current_dir_name);
    gpu_puts(dir_prompt);

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
            memset(dir_prompt, 0, sizeof(dir_prompt));
            dir_prompt = "";
            dir_prompt = strcat(dir_prompt, CLI_PROMPT);
            dir_prompt = strcat(dir_prompt, get_inode_name(current_dir_id));
            gpu_puts(dir_prompt);
            buf_index = 0;
        }

        
    } while (1);
}