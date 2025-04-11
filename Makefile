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
MEMORY_DIR = memory

# Files
ASM_FILES = $(SRC_DIR)multiboot.asm $(SRC_DIR)start.asm $(INTERRUPTS_DIR)/isr_stubs.asm
C_FILES = $(SRC_DIR)kernel.c \
          $(STD_DIR)/stdlib.c \
          $(STD_DIR)/assert.c \
          $(STD_DIR)/string.c \
          $(DRIVERS_DIR)/screen.c \
          $(DRIVERS_DIR)/keyboard.c \
          $(DRIVERS_DIR)/pit.c \
          $(SRC_DIR)shell.c \
          $(INTERRUPTS_DIR)/pic.c \
          $(INTERRUPTS_DIR)/idt.c \
          $(INTERRUPTS_DIR)/interupts_handler.c \
          $(SRC_DIR)gdt.c \
          $(DRIVERS_DIR)/disk.c \
          $(MEMORY_DIR)/utills.c \
          $(MEMORY_DIR)/vmm.c \
          $(MEMORY_DIR)/pmm.c \
          $(MEMORY_DIR)/kmalloc.c \
          $(SRC_DIR)/errors.c \
          $(STD_DIR)/stdio.c

OBJS = $(ASM_FILES:.asm=.o) $(C_FILES:.c=.o)

# Output
KERNEL = kernel.bin
ISO = kernel.iso
DISK_IMG = disk.img

.PHONY: all clean run debug create-disk

all: $(ISO) $(DISK_IMG)

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

# Create a raw disk image
$(DISK_IMG):
	qemu-img create -f raw $(DISK_IMG) 64M

# Clean build artifacts
clean:
	find . -name "*.o" -type f -delete
	rm -f $(KERNEL) $(ISO) $(DISK_IMG)
	rm -rf $(ISO_DIR)
	rm -rf $(GRUB_DIR)

# Run the kernel in QEMU with ATA PIO mode
run: $(ISO) $(DISK_IMG)
	qemu-system-i386 -cdrom kernel.iso -drive file=disk.img,format=raw,if=ide,index=0,media=disk -serial stdio

# Debug the kernel in QEMU with ATA PIO mode
debug: $(ISO) $(DISK_IMG)
	# Start QEMU in debugging mode (listening on port 1234)
	qemu-system-i386 -s -S -cdrom kernel.iso -drive file=disk.img,format=raw,if=ide,index=0,media=disk -serial stdio &
	# Open a new Windows console window that runs WSL bash to change directory and start GDB
	cmd.exe /C start bash -c "cd $(ISO_DIR) && gdb kernel.bin -ex 'target remote :1234'"

