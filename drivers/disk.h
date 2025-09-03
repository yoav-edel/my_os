#ifndef DISK_H
#define DISK_H

//todo add support for 48bit LBA and partitioning

/*
 * This file contains the definitions for the disk driver.
 * The disk driver support ATA PIO LBA28 mode for disk access.
 */
#include "../std/stdint.h"
#include "../std/stdbool.h"

#define DISK_NO_SLOT_AVAILABLE 0
// Register offsets from the base I/O port
#define ATA_REG_DATA          0x0
#define ATA_REG_ERR           0x1 // Error register (read) / Features register (write)
#define ATA_REG_SECCOUNT      0x2
#define ATA_REG_LBA_LOW       0x3
#define ATA_REG_LBA_MID       0x4
#define ATA_REG_LBA_HIGH      0x5
#define ATA_REG_DRIVE_SELECT  0x6
#define ATA_REG_CMD_STATUS    0x7
#define ATA_REG_ALTSTATUS     0x206 // Alternate status register

#define PRIMARY_BASE_PORT     0x1F0
#define SECONDARY_BASE_PORT   0x170

// ATA Commands
#define ATA_CMD_READ          0x20
#define ATA_CMD_WRITE         0x30
#define ATA_CMD_FLUSH         0xE7
#define ATA_CMD_IDENTIFY      0xEC

// ATA Status Flags
#define ATA_STATUS_BSY        0x80 // Busy
#define ATA_STATUS_DRQ        0x08 // Data request
#define ATA_STATUS_ERR        0x01 // Error

// Drive selection values
#define MASTER_DRIVE          0xE0 // Master drive, LBA mode
#define SLAVE_DRIVE           0xF0 // Slave drive, LBA mode

#define MAX_SECTORS_PER_CALL  256

// Identify Device Data Structure
typedef struct {
    uint16_t general_config;
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors_per_track;
    char serial_number[21];
    char firmware_revision[9];
    char model_number[41];
    uint16_t capabilities[2];
    uint32_t total_sectors;
    uint32_t logical_sector_size;
    uint16_t physical_sector_size;
    uint16_t base_io_port;
    bool slave; //todo combine the bools into a single byte
    bool valid; //indicates if the disk was successfully identified and can be used.
} identifyDeviceData;

/**
 * Initializes the disk driver for a given ATA channel and identifies the connected disk.
 *
 * @param base_port Base I/O port for the ATA channel (e.g., 0x1F0 for primary, 0x170 for secondary).
 */
void init_disk(uint16_t base_port);

void init_disk_driver();

/**
 * Identifies the specified drive on a given ATA channel and fills the provided identifyDeviceData struct.
 *
 * @param base_port Base I/O port for the ATA channel.
 * @param drive     Drive selection (MASTER_DRIVE or SLAVE_DRIVE).
 * @param data      Pointer to an identifyDeviceData structure to hold the results.
 * @return          true if the drive was successfully identified, false otherwise.
 */
bool identify_drive(uint16_t base_port, uint8_t drive, identifyDeviceData *data);

/*
 *
 */
bool ata_read_sectors(const uint8_t disk_num, const uint32_t lba_address, const uint8_t sector_count, void *buffer);

bool ata_write_sectors(uint8_t disk_num, const uint32_t lba_address, const uint8_t sector_count, void *buffer);

/**
 * Prints the Master Boot Record (MBR) of the specified disk.
 *
 * @param disk_num The disk number (0-3).
 */
//void print_mbr(uint8_t disk_num);

/*
 * print the first 512 bytes of the disk
 */
void test_disk_driver();

void switch_disk(uint8_t disk_num);

uint32_t disk_alloc_slot();

void disk_free_slot(uint32_t slot);

size_t disk_write(uint32_t addr, const void *buffer, const size_t len);

size_t disk_read(uint32_t addr, void *buffer, const size_t len);


#endif // DISK_H