#ifndef CLI_H
#define CLI_H

#include <stdint.h>

#define MAX_ROWS 25
#define MAX_COLS 80

#define CLI_PROMPT "betama@rasberrypi:~"

extern char* current_dir_name;

#define MAX_CMD_SIZE 20

// all commands must pass data param
typedef void (*cmd_t) (void* data);

typedef struct {
    char* name;
    cmd_t handler;
} cmd_entry_t;

// CURRENTLY COMMANDS SHOULD ONLY BE ONE WORD AND/OR ONE INPUT BESIDE
#define CMD_ENTRIES 7

extern cmd_entry_t cmd_entries[CMD_ENTRIES];

void register_cmd(const char* name, cmd_t cmd);

// HANDLE COMMANDS LIKE HOW INTERRUPTS ARE HANDLEED IN ISR.C
void handle_cmd(char buffer[]);

void shell_install();

void poll_cli_input();

#endif