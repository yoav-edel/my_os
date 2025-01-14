# Makefile for building the OS kernel

# Compiler and assembler
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

# Compiler and assembler flags
CFLAGS = -ffreestanding -O0 -g -nostdlib -c
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld

# Directories
SRC_DIR = ./
INTERRUPTS_DIR = interupts
STD_DIR = std
DRIVERS_DIR = drivers
BUILD_DIR = build
ISO_DIR = iso/boot
GRUB_DIR = $(ISO_DIR)/grub

# Files
ASM_FILES = $(SRC_DIR)multiboot.asm $(SRC_DIR)start.asm $(INTERRUPTS_DIR)/isr_stubs.asm
C_FILES = $(SRC_DIR)kernel.c \
          $(STD_DIR)/stdlib.c \
          $(STD_DIR)/assert.c \
          $(STD_DIR)/string.c \
          $(DRIVERS_DIR)/screen.c \
          $(DRIVERS_DIR)/keyboard.c \
          $(SRC_DIR)shell.c \
          $(INTERRUPTS_DIR)/pic.c \
          $(INTERRUPTS_DIR)/idt.c \
          $(INTERRUPTS_DIR)/interupts_handler.c \
          $(SRC_DIR)gdt.c
OBJS = $(ASM_FILES:.asm=.o) $(C_FILES:.c=.o)

# Output
KERNEL = kernel.bin
ISO = kernel.iso

.PHONY: all clean run debug

all: $(ISO)

# Compile assembly files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Link the kernel
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create the ISO
$(ISO): $(KERNEL)
	mkdir -p $(GRUB_DIR)
	cp $(KERNEL) $(ISO_DIR)/
	@echo "set timeout=0" > $(GRUB_DIR)/grub.cfg
	@echo "set default=0" >> $(GRUB_DIR)/grub.cfg
	@echo "menuentry \"My Kernel\" {" >> $(GRUB_DIR)/grub.cfg
	@echo "    multiboot /boot/$(KERNEL)" >> $(GRUB_DIR)/grub.cfg
	@echo "    boot" >> $(GRUB_DIR)/grub.cfg
	@echo "}" >> $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $@ iso

# Clean build artifacts
clean:
	find . -name "*.o" -type f -delete
	rm -f $(KERNEL) $(ISO)
	rm -rf $(ISO_DIR)
	rm -rf $(GRUB_DIR)

# Run the kernel in QEMU
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

# Debug the kernel in QEMU
debug: $(ISO)
	qemu-system-i386 -s -S -cdrom $(ISO)
