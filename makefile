# Define directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Define files and flags
LINKER_SCRIPT = $(BUILD_DIR)/linker.ld
TARGET = myos.elf
CC = arm-linux-gnueabihf-gcc
CFLAGS = -mcpu=cortex-a7 -fPIC -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -nostdlib -T $(LINKER_SCRIPT)

# Collect source files
SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.S)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJECTS := $(OBJECTS:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

# Default rule
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Assemble assembly files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean

