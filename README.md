# BetamaBridgeOS

A bare-metal OS prototype targeting the **Raspberry Pi 2 (ARM)**. It boots from assembly into a C kernel, initializes UART + a simple memory manager, sets up a framebuffer-backed text console via the GPU mailbox interface, and exposes a small **interactive shell** with a **toy inode/zone filesystem** that prints detailed debug logs during file and directory creation.

## Demo

YouTube: https://youtu.be/jcRRI7HMuiI

> Tip: In the demo, the **right window** is the OS shell, and the **left window** shows filesystem/kernel debug output (inode address, mode, zones, directory entries, etc.).

---

## Features

### Boot + kernel bring-up
- Minimal `boot.S` that sets the stack, clears `.bss`, and jumps into `kernel_main`.

### Console I/O
- **UART input** for shell commands.
- **Framebuffer text output** (GPU mailbox/property channel) so output is visible even when not relying on serial-only.

### Memory management
- Basic page allocator + kernel heap allocator (best-fit style).  
  Enough to support dynamic allocations used by the shell and filesystem.

### Shell (CLI)
Implemented commands:
- `echo <text>` — print text
- `pwd` — print current directory
- `ls` — list entries in the current directory (prints inode addresses + names)
- `cd <dir>` — change directory
- `mkdir <name>` — create a directory
- `touch <name>` — create an empty file
- `cat <file>` — print file contents
- `write <file> <text>` — creates a file with content if it doesn’t exist (rewrite is currently not enabled)

### Filesystem (SIM_FS)
- In-memory “disk” with:
  - Superblock (stored at block 1)
  - Inode metadata table (stored at block 2)
  - Bitmaps for inode and zone allocation
- Inodes track:
  - mode (directory vs regular file + permissions flags)
  - link count
  - up to **3 direct zones**
- **Verbose logging** on create:
  - inode address
  - mode
  - file size / link count
  - allocated zones
  - directory entries / file data preview (where applicable)

---

## Project layout (high-level)

- `boot.S` — early boot / entrypoint
- `kernel.c` — kernel main sequence (init + shell loop)
- `uart.c` — UART init + non-blocking reads
- `memory.c` — allocator + low-level memory utils
- `framebuffer.c`, `gpu.c` — GPU mailbox/property calls + framebuffer setup
- `cli.c` — shell command parsing + handlers
- `fs.c` — simulated filesystem implementation
- `spi.c`, `sd.c` — SPI/SD plumbing (present; may be WIP depending on your branch)

---

## How the demo maps to the implementation

1) **Boot → kernel init**  
   `boot.S` → `kernel_main()` initializes UART, memory, filesystem, framebuffer.

2) **Shell command loop**  
   `shell_install()` registers commands; `poll_cli_input()` reads characters via UART and dispatches handlers.

3) **File/directory creation with logs**  
   `mkdir` / `touch` → filesystem allocates an inode + zones, updates the current directory’s entry list, then prints a detailed creation log.

---

## Build & run (remote compile → push `myos.elf` → run on Raspberry Pi 2)

This project is **cross-compiled on a development machine**, then the produced `myos.elf` is copied to a Raspberry Pi 2 over SSH for execution.

### Prerequisites (development machine)
- ARM cross-compiler toolchain (the Makefile builds `myos.elf`)
- `make`
- `rsync` (or `scp`)
- SSH access to the Pi

### Build + deploy (manual)
```bash
# From the repo root:
make clean
make SIM_FS=1 QEMU=1

# Copy the ELF to the Pi (edit host/path as needed):
rsync -avz myos.elf pi@<PI_HOST>:~/betama/ServerBuild/

### Run on the Raspberry Pi 2 (QEMU emulator)
```bash
# SSH into the Pi:
ssh pi@<PI_HOST>
cd ~/betama/ServerBuild

# Run QEMU in "raspi2" machine mode, booting the ELF as the kernel.
# -nographic sends UART to your terminal.
qemu-system-arm -M raspi2 -kernel myos.elf -nographic
```

---

## Heap + in-memory filesystem (summary)

This project is a **learning OS** built to practice kernel bring-up, memory allocation, and filesystem fundamentals. Because SD read/write under **QEMU** was unreliable during development, the filesystem is intentionally **simulated in RAM** so the focus stays on core FS logic and debugging—not hardware quirks.

### Kernel heap (dynamic allocation)
- Uses a simple **page allocator** (4KB pages) plus a small **heap allocator** (`kmalloc`/`kfree`) for dynamic memory needed by the CLI + filesystem.
- Designed for correctness in the demo and learning value, not production robustness.

### Simulated filesystem (SIM_FS)
- The “disk” is a fixed **RAM region** treated like a block device (1KB blocks).
- Implements a lightweight inode/zone design:
  - inode + zone **bitmaps** for allocation
  - **directories** as fixed-size directory entries stored in data zones
  - **direct zones** per inode (small, intentionally simple)
- Prints verbose logs during `mkdir` / `touch` showing **inode metadata + zone allocation**, which is what you see in the demo.

### Key limitation
- **No persistence**: the filesystem resets on reboot because storage is RAM-backed.

---


