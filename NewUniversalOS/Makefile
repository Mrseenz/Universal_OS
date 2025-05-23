NASM = nasm
QEMU = qemu-system-i386

BUILD_DIR = build
SRC_DIR = src

BOOT_SRC = $(SRC_DIR)/boot.asm
BOOT_BIN = $(BUILD_DIR)/boot.bin

.PHONY: all clean run

all: $(BOOT_BIN)

$(BOOT_BIN): $(BOOT_SRC)
	@mkdir -p $(BUILD_DIR)
	$(NASM) $(BOOT_SRC) -f bin -o $(BOOT_BIN)

run: $(BOOT_BIN)
	$(QEMU) -fda $(BOOT_BIN)

clean:
	@rm -rf $(BUILD_DIR)
