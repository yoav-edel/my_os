//
// Created by Yoav on 12/3/2024.
//

#ifndef MYKERNELPROJECT_DISK_H
#define MYKERNELPROJECT_DISK_H

#include <stdint.h>
#include <stdbool.h>

#define NO_SLOT_AVAILABLE 0xFFFFFFFF

// slot size is 4KB and are aligned to 4KB
/*
 * The function allocates a slot in the disk
 */
uint32_t disk_alloc_slot();
void disk_free_slot(uint32_t slot);
/*
 * The function read the data to the slot starting from the offset.
 * return true if the write was successful, false otherwise
 * //todo add checks that offset and size are in the slot range. if not return an error
 */
bool disk_read(uint32_t slot, uint16_t offset, uint8_t *buffer, uint32_t size);
/*
 * The function writes the data to the slot starting from the offset.
 * return true if the write was successful, false otherwise
 * //todo add checks that offset and size are in the slot range. if not return an error
 */
bool disk_write(uint32_t slot, uint16_t offset, uint8_t *buffer, uint32_t size);


#endif //MYKERNELPROJECT_DISK_H
