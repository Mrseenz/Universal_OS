NASM = nasm
# Using host gcc/ld/objcopy with multilib support
CC = gcc
LD = ld
OBJCOPY = objcopy
CAT = cat

# CFLAGS for compiling C code, added -I$(SRC_DIR) for local includes
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32 -nostdlib -fno-stack-protector -fno-pie -I$(SRC_DIR)
# LDFLAGS for linking the kernel - -m elf_i386 for host ld
LDFLAGS = -m elf_i386
# NASMFLAGS for assembling .asm files to ELF objects
NASMFLAGS = -f elf32
# OBJCOPYFLAGS for raw binary output
OBJCOPYFLAGS = -O binary

QEMU = qemu-system-i386

BUILD_DIR = build
SRC_DIR = src

# Bootloader source
BOOT_SRC = $(SRC_DIR)/boot.asm
BOOT_BIN = $(BUILD_DIR)/boot.bin

# Kernel source files
KERNEL_C_SOURCES = $(SRC_DIR)/kernel.c $(SRC_DIR)/interrupts.c $(SRC_DIR)/vga_text.c $(SRC_DIR)/pit.c $(SRC_DIR)/serial.c
KERNEL_ASM_SOURCES = $(SRC_DIR)/idt.asm $(SRC_DIR)/graphics.asm

# Kernel object files (derived from sources using patsubst)
KERNEL_C_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SOURCES))
KERNEL_ASM_OBJS = $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SOURCES))
KERNEL_OBJS = $(KERNEL_C_OBJS) $(KERNEL_ASM_OBJS)

# Kernel output files
KERNEL_LD_SCRIPT = $(SRC_DIR)/linker.ld
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# Final OS image
OS_IMAGE = $(BUILD_DIR)/os_image.bin

.PHONY: all clean run

all: $(OS_IMAGE)

# Rule for bootloader (remains specific)
$(BOOT_BIN): $(BOOT_SRC)
	@mkdir -p $(@D)
	$(NASM) $< -f bin -o $@

# Generic rule for compiling C source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Generic rule for assembling NASM source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) $< -o $@

# Rule for linking kernel object files into an ELF file
$(KERNEL_ELF): $(KERNEL_OBJS) $(KERNEL_LD_SCRIPT)
	$(LD) $(LDFLAGS) $(KERNEL_LD_SCRIPT) $(KERNEL_OBJS) -o $(KERNEL_ELF)

# Rule for converting kernel ELF to binary
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(KERNEL_ELF) $(KERNEL_BIN)

# Rule for creating the final OS image
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	$(CAT) $(BOOT_BIN) $(KERNEL_BIN) > $(OS_IMAGE)

# Rule to run QEMU (serial to stdio, no graphics, monitor to null)
run: all
	$(QEMU) -fda $(OS_IMAGE) -serial stdio -nographic -monitor null

clean:
	@rm -rf $(BUILD_DIR)/*
	# Note: No need to explicitly rm $(OS_IMAGE) if it's in $(BUILD_DIR)
	# The example clean target was just `@rm -rf $(BUILD_DIR)/*`
	# which is what I've used.
