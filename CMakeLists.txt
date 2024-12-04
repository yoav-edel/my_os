cmake_minimum_required(VERSION 3.0)
project(MyKernelProject)

# Add a target to build the bootloader (assembly file)
add_custom_target(bootloader ALL
        COMMAND nasm -f bin ${CMAKE_SOURCE_DIR}/bootloader.asm -o bootloader.bin
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add a target to compile the kernel (C file)
add_executable(kernel kernel.c
        kernel.c
        main.c
        drivers/screen.c
        drivers/screen.h
        interupts/idt.c
        interupts/idt.h
        gdt.h
        gdt.c
        interupts/interupts_handler.h
        interupts/interupts_handler.c
        interupts/pic.h
        interupts/pic.c
        drivers/io.h
        drivers/keyboard.h
        drivers/keyboard.c
        memory/vmm.h
        memory/vmm.c
        memory/pmm.c
        memory/pmm.h
        memory/utills.h
        memory/utills.c
        errors.h
        drivers/disk.c
        drivers/disk.h)

# Combine the bootloader and kernel into a bootable image
add_custom_target(kernel_image ALL
        COMMAND cat bootloader.bin kernel > boot.img
        DEPENDS bootloader kernel
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Optional: Specify build flags for the kernel
set_target_properties(kernel PROPERTIES LINK_FLAGS "-Ttext 0x1000 --oformat binary")
