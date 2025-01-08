cmake_minimum_required(VERSION 3.16)

# Project name and version
project(MyKernel VERSION 1.0 LANGUAGES C ASM_NASM)

# Cross compiler setup
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_COMPILER i686-elf-gcc)
set(CMAKE_ASM_NASM_COMPILER nasm)

# Compiler flags
set(CMAKE_C_FLAGS "-ffreestanding -O2 -nostdlib")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib")

# Source files
set(SRC_FILES
        ${CMAKE_SOURCE_DIR}/kernel.c
        ${CMAKE_SOURCE_DIR}/multiboot.asm
        ${CMAKE_SOURCE_DIR}/start.asm
        ${CMAKE_SOURCE_DIR}/std/stdlib.c
        ${CMAKE_SOURCE_DIR}/drivers/screen.c
        shell.c
        shell.h
        std/stdbool.h
)

# Include directories
include_directories(
        ${CMAKE_SOURCE_DIR}/std
        ${CMAKE_SOURCE_DIR}/drivers
)

# Output kernel binary
add_executable(kernel.bin ${SRC_FILES})

# Specify NASM rules for .asm files
set_source_files_properties(multiboot.asm start.asm PROPERTIES LANGUAGE ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32) # Ensure NASM uses ELF32 format

# Post-build step: Convert to ISO
add_custom_command(TARGET kernel.bin POST_BUILD
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/isodir/boot/grub
        COMMAND cp kernel.bin ${CMAKE_BINARY_DIR}/isodir/boot/
        COMMAND echo "set timeout=0" > ${CMAKE_BINARY_DIR}/isodir/boot/grub/grub.cfg
        COMMAND echo "set default=0" >> ${CMAKE_BINARY_DIR}/isodir/boot/grub/grub.cfg
        COMMAND echo "menuentry \"My Kernel\" {" >> ${CMAKE_BINARY_DIR}/isodir/boot/grub/grub.cfg
        COMMAND echo "  multiboot /boot/kernel.bin" >> ${CMAKE_BINARY_DIR}/isodir/boot/grub.cfg
        COMMAND echo "}" >> ${CMAKE_BINARY_DIR}/isodir/boot/grub.cfg
        COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/kernel.iso ${CMAKE_BINARY_DIR}/isodir
)

# Add a custom target to run QEMU
add_custom_target(run
        COMMAND qemu-system-i386 -cdrom ${CMAKE_BINARY_DIR}/kernel.iso
        DEPENDS kernel.bin
)
