# Define directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Define files and flags
LINKER_SCRIPT = $(BUILD_DIR)/linker.ld
TARGET = myos.elf
CC = arm-linux-gnueabihf-gcc
CFLAGS = -mcpu=cortex-a7 -fPIC -ffreestanding -g -O0 -Wall -Wextra # O0 is used to disable compiler optimizations for the code

# Detect if running in QEMU
ifeq ($(QEMU),1)
    CFLAGS += -DQEMU
endif

LDFLAGS = -ffreestanding -nostdlib -T $(LINKER_SCRIPT)

KERNEL_SRC = $(SRC_DIR)/kernel
COMMON_SRC = $(SRC_DIR)/common

# Collect source files
#KERNEL_SOURCES = $(wildcard $(KERNEL_SRC)/*.c)
#KERNEL_ASM_SOURCES = $(wildcard $(KERNEL_SRC)/*.S)
#COMMON_SOURCES = $(wildcard $(COMMON_SRC)/*.c)
KERNEL_HEAD = $(INCLUDE_DIR)/kernel
COMMON_HEAD = $(INCLUDE_DIR)/common


#OBJECTS = $(KERNEL_SOURCES:$(KERNEL_SRC)/%.c=$(BUILD_DIR)/%.o)

#OBJECTS += $(KERNEL_ASM_SOURCES:$(KERNEL_SRC)/%.c=$(BUILD_DIR)/%.o)
#OBJECTS += $(COMMON_SOURCES:$(COMMON_SRC)/%.c=$(BUILD_DIR)/%.o)

KERNEL_SOURCES = $(wildcard $(KERNEL_SRC)/*.c)
KERNEL_ASM_SOURCES = $(wildcard $(KERNEL_SRC)/*.S)
COMMON_SOURCES = $(wildcard $(COMMON_SRC)/*.c)

# Generate object files
OBJECTS = $(patsubst $(KERNEL_SRC)/%.c, $(BUILD_DIR)/kernel_%.o, $(KERNEL_SOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.c, $(BUILD_DIR)/common_%.o, $(COMMON_SOURCES))
OBJECTS += $(patsubst $(KERNEL_SRC)/%.S, $(BUILD_DIR)/kernel_%.o, $(KERNEL_ASM_SOURCES))




# Default rule
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Compile C source files from kernel directory
$(BUILD_DIR)/kernel_%.o: $(KERNEL_SRC)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(KERNEL_HEAD) -I$(COMMON_HEAD) -c $< -o $@

# Compile C source files from common directory
$(BUILD_DIR)/common_%.o: $(COMMON_SRC)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(KERNEL_HEAD) -I$(COMMON_HEAD) -c $< -o $@

# Assemble assembly files from kernel directory
$(BUILD_DIR)/kernel_%.o: $(KERNEL_SRC)/%.S
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(KERNEL_HEAD) -I$(COMMON_HEAD) -c $< -o $@

# Clean rule
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean

