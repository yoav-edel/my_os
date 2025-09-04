#include "disk.h"
#include "io.h"
#include "screen.h"
#include "../memory/utills.h"
#include "../memory/vmm.h"
#include "../std/stdio.h"
/*
 * Explanation about the delay that appears sometimes in the code:
 * According to the ATA specifications, after selecting a new drive (Master/Slave),
 * a short delay of 400ns is required before reading the Status Register.
 * This is because some drives take time to assert their status onto the bus.
 * The recommended approach is to read the Status Register 15 times, but only use the last value.
 * Since each I/O read takes about 30ns, this results in a 420ns delay, allowing the drive enough time to update.
 * Also, after sending a command to the Command Register, it is recommended to read the Alternate Status Register
 * four times because, of course, whoever designed it made it possible that ERR or DF bits are incorrect just to
 * complicate things further. If you find a place where I put the delay and you think it's not needed, please let me know.

 */



// TODO: Add support when ERR flag is set.
/* TODO: Support multiple disks with page swapping - approaches can be to write to one of the reserved bits of the
 page_entry for the disk number or only write to a single disk */

static identifyDeviceData disk1 = {0};
static identifyDeviceData disk2 = {0};
static identifyDeviceData disk3 = {0};
static identifyDeviceData disk4 = {0};

static identifyDeviceData *disks[4] = {&disk1, &disk2, &disk3, &disk4};

#define DISK_BITMAP_SIZE 100000 // TODO: Change this to be dynamic with kmalloc and the size in the disks
static uint8_t disk_bitmap[DISK_BITMAP_SIZE] = {0};

inline static bool is_slot_free(const uint32_t slot) {
    return !(disk_bitmap[slot / 8] & (1 << (slot % 8)));
}

inline static void disk_mark_used(const uint32_t slot) {
    disk_bitmap[slot / 8] |= (1 << (slot % 8));
}

static inline bool are_slots_free(const uint32_t start, const uint32_t count) {
    const uint32_t limit = DISK_BITMAP_SIZE * 8;
    if (start + count > limit)
        return false;
    for (uint32_t k = 0; k < count; k++) {
        if (!is_slot_free(start + k)) return false;
    }
    return true;
}


static inline void mark_slots_used(const uint32_t start, const uint32_t count) {
    for (uint32_t k = 0; k < count; k++) disk_mark_used(start + k);
}

inline static void disk_mark_free(const uint32_t slot) {
    disk_bitmap[slot / 8] = disk_bitmap[slot / 8] & ~(1 << (slot % 8));
}


/*
 * Free a previously allocated disk slot.
 */
void disk_free_slot(const uint32_t slot) {
    disk_mark_free(slot);
}

void disk_free_slots(const uint32_t slot, const uint8_t slots_num) {
    if (slot + slots_num >= DISK_BITMAP_SIZE * 8) return;
    for (uint8_t i = 0; i < slots_num; i++)
        disk_mark_free(slot + i);
}

// allocate via next fit algorithm


static size_t last_alloc_index = 1;


uint32_t disk_alloc_slots(const uint8_t slots_num) {
    if (slots_num == 0) return DISK_NO_SLOT_AVAILABLE;

    // Next-fit: from last_alloc_index to end (byte-wise), then wrap
    for (size_t i = last_alloc_index; i < DISK_BITMAP_SIZE; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            const uint32_t slot = (uint32_t) i * 8 + j;
            if (!is_slot_free(slot)) continue;

            if (are_slots_free(slot, slots_num)) {
                mark_slots_used(slot, slots_num);
                last_alloc_index = (slot + slots_num) / 8;
                return slot;
            }
        }
    }
    for (size_t i = 0; i < last_alloc_index; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            const uint32_t slot = (uint32_t) i * 8 + j;
            if (!is_slot_free(slot)) continue;

            if (are_slots_free(slot, slots_num)) {
                mark_slots_used(slot, slots_num);
                last_alloc_index = (slot + slots_num) / 8;
                return slot;
            }
        }
    }
    return DISK_NO_SLOT_AVAILABLE;
}



/*
 * The following functions are used to extract the LBA address into its components.
 * The lba address is 24 bits long and is divided into 4 parts:
 * 1. The lowest 8 bits
 * 2. The next 8 bits
 * 3. The next 8 bits
 * 4. The highest 4 bits
 *
 */
static inline uint8_t get_lba_low(const uint32_t lba) {
    return (uint8_t)lba;
}

static inline uint8_t get_lba_mid(const uint32_t lba) {
    return (uint8_t)(lba >> 8);
}

static inline uint8_t get_lba_high(const uint32_t lba) {
    return (uint8_t)(lba >> 16);
}


static inline uint8_t get_lba_highest(const uint32_t lba) {
    return (uint8_t)(lba >> 24) & 0x0F;
}

static inline bool is_lba_supported(const identifyDeviceData *disk) {
    return disk->capabilities[0] & (1 << 9);
}

/*
 * The following function waits for the BSY flag in the status register to clear, with a timeout.
 * The BSY flag indicates that the drive is busy and cannot accept commands.
 * If the BSY flag clears, the function returns true.
 */
static bool ata_wait_for_bsy(uint16_t base_port) {
    for (int timeout = 0; timeout < 1000000000; timeout++) {
        if (!(inb(base_port + ATA_REG_CMD_STATUS) & ATA_STATUS_BSY)) {
            return true;
        }
    }
    return false;
}




/*
 * The following function waits for the DRQ flag in the status register to set, with a timeout.
 * The DRQ flag indicates that the drive is ready to transfer data.
 * If the ERR flag is set, an error occurred and the function returns false.
 * If the DRQ flag is set, the function returns true.
 * If the timeout is reached, the function returns false.
 *
 * @param base_port The base I/O port for the ATA channel.
 * @return true if the DRQ flag is set, false if an error occurred or the timeout was reached.
 */
static bool ata_wait_for_drq(uint16_t base_port) {
    for (int timeout = 0; timeout < 1000000; timeout++) {
        uint8_t status = inb(base_port + ATA_REG_CMD_STATUS);
        if (status & ATA_STATUS_ERR) {
            return false; // An error occurred
        }
        if (status & ATA_STATUS_DRQ) {
            return true; // Ready for data transfer
        }
    }
    return false;
}

// Extracts and formats an ATA string from the raw data.
static void ata_extract_string(const uint16_t *source, char *dest, int length) {
    for (int i = 0; i < length / 2; i++) {
        dest[i * 2] = (char)(source[i] >> 8);   // High byte
        dest[i * 2 + 1] = (char)(source[i] & 0xFF); // Low byte
    }
    dest[length] = '\0';

    // Remove trailing spaces
    for (int i = length - 1; i >= 0; i--) {
        if (dest[i] == ' ' || dest[i] == '\0') {
            dest[i] = '\0';
        } else {
            break;
        }
    }
}

static inline void delay400ns(uint16_t base_port) {
    for (int i = 0; i < 14; i++) {
        (void)inb(base_port + ATA_REG_ALTSTATUS);
    }
}

/*
 * This function introduces a delay after writing a command to the Command Register when using pooling.
 * It reads the Alternate Status Register four times to ensure that the ERR or DF bits,
 * which may still be set accidentally, are cleared. This is necessary because the first
 * four reads of the Status Register after sending a command byte may have the ERR or DF
 * bits still set.
 */
static inline void delayAfterCommand(uint16_t base_port) {
    for (int i = 0; i < 4; i++) {
        (void)inb(base_port + ATA_REG_ALTSTATUS);
    }
}

/*
 * the function makes sure that the data has been written to the disk from the cache.
 */
inline static void flush_cache(uint16_t base_port) {
    outb(base_port + ATA_REG_CMD_STATUS, ATA_CMD_FLUSH);
    delayAfterCommand(base_port);
}


uint32_t parse_logical_sector_size(const uint16_t *identify_data) {
    // Word 106 - Physical/Logical Sector Size Information
    uint16_t word_106 = identify_data[106];

    // Check if Word 106 is valid (Bit 14 must be set)
    if (!(word_106 & (1 << 14))) {
        // TODO: Raise panic because this is not correct
        return 512;
    }

    // Check if the device supports logical sector sizes larger than 256 words (Bit 12)
    if (word_106 & (1 << 12)) {
        // Logical Sector Size Supported - Retrieve Words 117 and 118
        uint16_t word_117 = identify_data[117];
        uint16_t word_118 = identify_data[118];

        // Combine Words 117 and 118 to get the logical sector size in words
        uint32_t logical_sector_size_in_words = ((uint32_t)word_118 << 16) | word_117;

        // Convert the size to bytes (1 word = 2 bytes)
        return logical_sector_size_in_words * 2;
    }

    // TODO: Raise panic because I don't know what to do
    return 512;
}


// Parse Physical Sector Size
uint32_t parse_physical_sector_size(const uint16_t *identify_data) {
    uint16_t word_106 = identify_data[106];

    // Check if the Logical-to-Physical Sector Relationship Supported bit (Bit 3:0) is valid
    if (word_106 & (1 << 3)) {
        uint8_t logical_to_physical_ratio = word_106 & 0x0F; // Bits 3:0 indicate the ratio
        uint32_t logical_sector_size = parse_logical_sector_size(identify_data);

        // Calculate physical sector size
        return logical_sector_size << logical_to_physical_ratio;
    }

    // Default physical sector size equals logical sector size
    return parse_logical_sector_size(identify_data);
}


// Identifies the specified drive and extracts information into identifyDeviceData.
bool identify_drive(const uint16_t base_port, const uint8_t drive, identifyDeviceData *data) {
    uint16_t identify_data[256];

    // Select the drive
    outb(base_port + ATA_REG_DRIVE_SELECT, drive);

    delay400ns(base_port);

    // Clear the Sector Count and LBA registers
    outb(base_port + ATA_REG_SECCOUNT, 0);
    outb(base_port + ATA_REG_LBA_LOW, 0);
    outb(base_port + ATA_REG_LBA_MID, 0);
    outb(base_port + ATA_REG_LBA_HIGH, 0);

    // Send the IDENTIFY command
    outb(base_port + ATA_REG_CMD_STATUS, ATA_CMD_IDENTIFY);

    // 400ns delay before reading the status register - because the drive needs time to process the command
    delay400ns(base_port);

    // Check if the drive exists
    if (inb(base_port + ATA_REG_CMD_STATUS) == 0) {
        printf("Drive does not exist.\n");
        return false;
    }

    // Wait for BSY to clear
    if (!ata_wait_for_bsy(base_port)) {
        printf("Timeout waiting for BSY=0.\n");
        return false;
    }

    // Check if this is an ATA drive
    if (inb(base_port + ATA_REG_LBA_MID) != 0 || inb(base_port + ATA_REG_LBA_HIGH) != 0) {
        printf("Not an ATA drive.\n");
        return false;
    }

    // Wait for DRQ to set
    if (!ata_wait_for_drq(base_port)) {
        printf("Timeout waiting for DRQ=1.\n");
        return false;
    }

    // Read 256 words from the data register
    for (int i = 0; i < 256; i++) {
        identify_data[i] = in16(base_port + ATA_REG_DATA);
    }

    // Populate the identifyDeviceData structure
    data->general_config = identify_data[0];
    data->cylinders = identify_data[1];
    data->heads = identify_data[3];
    data->sectors_per_track = identify_data[6];
    ata_extract_string(&identify_data[10], data->serial_number, 20);
    ata_extract_string(&identify_data[23], data->firmware_revision, 8);
    ata_extract_string(&identify_data[27], data->model_number, 40);
    data->capabilities[0] = identify_data[49];
    data->capabilities[1] = identify_data[50];
    data->total_sectors = identify_data[60] | ((uint32_t)identify_data[61] << 16);
    data->logical_sector_size = parse_logical_sector_size(identify_data);
    data->physical_sector_size = parse_physical_sector_size(identify_data);
    data->base_io_port = base_port;
    data->slave = (drive == SLAVE_DRIVE);
    data->valid = true;
    return true;
}

/*
 * Reads sectors from the specified disk into the provided buffer.
 * Explanations about how the reading operation works:
 * 1. Because we are using a polling strategy, it waits for the BSY flag to clear.
 * 2. It sends the drive selection and LBA address to the disk.
 * 3. It sends the READ SECTORS command to the disk.
 * 4. It waits for the BSY flag to clear and the DRQ flag to set.
 * 5. It reads the data from the disk into the buffer.
 * 6. It repeats the process for the specified number of sectors.
 * 7. It returns true if the operation was successful, false otherwise.
 *
 * @param disk_num      The disk number (0-3).
 * @param lba_address   The starting LBA address.
 * @param sector_count  The number of sectors to read. If 0, 256 sectors are read.
 * @param buffer        The buffer to store the data.
 * @return              true if the operation was successful, false otherwise.
 */
bool ata_read_sectors(const uint8_t disk_num, const uint32_t lba_address, const uint8_t sector_count, void *buffer) {
    if (disk_num >= sizeof(disks) / sizeof(disks[0]))
        return false;

    identifyDeviceData *disk = disks[disk_num];
    if (!disk->valid || !is_lba_supported(disk))
        return false;

    if(lba_address + sector_count >= disk->total_sectors)
        return false;

    uint16_t base_port = disk->base_io_port;
    uint8_t drive = disk->slave ? SLAVE_DRIVE : MASTER_DRIVE;

    // we are using pooling so we need to wait for the busy flag to clear
    if (!ata_wait_for_bsy(base_port)) {
        printf("Timeout: Drive stuck busy.\n");
        return false;
    }

    // TODO: Add optimization to check which drive is currently selected and only change if
    // needed to avoid the delay400ns.
    // Send the highest 4 bits of the LBA, ORed with the drive selection
    outb(base_port + ATA_REG_DRIVE_SELECT, drive | get_lba_highest(lba_address));
    delay400ns(base_port);


    // Send the sector count
    outb(base_port + ATA_REG_SECCOUNT, sector_count);

    // Send the low 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_LOW, get_lba_low(lba_address));

    // Send the next 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_MID, get_lba_mid(lba_address));

    // Send the next 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_HIGH, get_lba_high(lba_address));

    // Send the READ SECTORS command
    outb(base_port + ATA_REG_CMD_STATUS, ATA_CMD_READ);

    // Delay after sending the command
    delayAfterCommand(base_port);
    // TODO: I think that for my implementation it's not needed because I wait for the flags.

    uint16_t *buffer16 = (uint16_t*)buffer;
    for (uint16_t i = 0; i < sector_count; i++) {
        // Wait for BSY to clear and DRQ to set
        if (!ata_wait_for_bsy(base_port) || !ata_wait_for_drq(base_port))
            return false;



        for (int j = 0; j < disk->logical_sector_size / 2; j++)
            buffer16[j] = in16(base_port + ATA_REG_DATA);

        // 400ns delay
        buffer16 += disk->logical_sector_size / 2;
        delay400ns(base_port);
    }

    return true;
}

/*
 * Writes sectors to the specified disk from the provided buffer.
 * Explanations about how the writing operation works:
 * 1. Because we are using a polling strategy, it waits for the BSY flag to clear.
 * 2. It sends the drive selection and LBA address to the disk.
 * 3. It sends the sector count to the disk.
 * 4. It sends the WRITE SECTORS command to the disk.
 * 5. It waits for the BSY flag to clear and the DRQ flag to set.
 * 6. It writes the data from the buffer to the disk.
 * 7. It repeats the process for the specified number of sectors.
 * 8. It returns true if the operation was successful, false otherwise.
 * @param disk_num      The disk number (0-3).
 * @param lba_address   The starting LBA address.
 * @param sector_count  The number of sectors to write. If 0, 256 sectors are written.
 * @param buffer        The buffer containing the data.
 * @return              true if the operation was successful, false otherwise.
 */
bool ata_write_sectors(uint8_t disk_num, const uint32_t lba_address, const uint8_t sector_count, void *buffer) {
    if (disk_num >= sizeof(disks) / sizeof(disks[0]))
        return false;

    identifyDeviceData *disk = disks[disk_num];
    if (!disk->valid || !is_lba_supported(disk))
        return false;

    if (lba_address + sector_count >= disk->total_sectors)
        return false;


    uint16_t base_port = disk->base_io_port;
    uint8_t drive = disk->slave ? SLAVE_DRIVE : MASTER_DRIVE;

    // we are using pooling so we need to wait for the busy flag to clear
    while (!ata_wait_for_bsy(base_port));

    // TODO: Add optimization to check which drive is currently selected and only change if needed
    // to avoid the delay400ns.
    // Send the highest 4 bits of the LBA, ORed with the drive selection
    outb(base_port + ATA_REG_DRIVE_SELECT, drive | get_lba_highest(lba_address));
    delay400ns(base_port);

    // Send the sector count
    outb(base_port + ATA_REG_SECCOUNT, sector_count);

    // Send the low 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_LOW, get_lba_low(lba_address));

    // Send the next 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_MID, get_lba_mid(lba_address));

    // Send the next 8 bits of the LBA
    outb(base_port + ATA_REG_LBA_HIGH, get_lba_high(lba_address));

    outb(base_port + ATA_REG_CMD_STATUS, ATA_CMD_WRITE);
    delayAfterCommand(base_port);

    const uint16_t *buffer16 = (uint16_t *) buffer;
    for (uint16_t i = 0; i < sector_count; i++) {
        // Wait for BSY to clear and DRQ to set
        if (!ata_wait_for_bsy(base_port) || !ata_wait_for_drq(base_port))
            return false;

        for (int j = 0; j < disk->logical_sector_size / 2; j++)
            out16(base_port + ATA_REG_DATA, buffer16[j]);

        buffer16 += disk->logical_sector_size / 2;
        asm volatile("jmp 1f\n\t" "1:");  // small delay to allow the drive to finish the operation
        flush_cache(base_port);
    }

    return true;


}

void init_disk_driver(){
    uint8_t num_drives = 0;
    num_drives += identify_drive(PRIMARY_BASE_PORT, MASTER_DRIVE, &disk1);
    num_drives += identify_drive(PRIMARY_BASE_PORT, SLAVE_DRIVE, &disk2);
    num_drives += identify_drive(SECONDARY_BASE_PORT, MASTER_DRIVE, &disk3);
    num_drives += identify_drive(SECONDARY_BASE_PORT, SLAVE_DRIVE, &disk4);
    if(!num_drives)
    {
        // todo raise panic
    }

    for (int i = 0; i < 4; i++) {
        identifyDeviceData *disk = disks[i];
        if (disk->valid) {
            printf("Disk %d Model: %s\n", i, disk->model_number);
            printf("Serial: %s\n", disk->serial_number);
            printf("Firmware: %s\n", disk->firmware_revision);
            printf("Total Sectors: %d\n", disk->total_sectors);
            printf("logical sector size: %d\n", disk->logical_sector_size);
            printf("physical sector size: %d\n", disk->physical_sector_size);
            printf("Base I/O Port: %d\n", disk->base_io_port);
            printf("Slave: %d\n\n", disk->slave);
        }
    }
}

// ------------------------------------------------------------
// Wrapper functions for the disk driver, to allow easy access
static uint8_t curr_disk = 0;

void switch_disk(uint8_t num) {
    if (num < 4 && disks[num] && disks[num]->valid)
        curr_disk = num;
}

/*
 * Read len bytes from the current disk (assumed to be disks[curr_disk])
 * starting at logical block address addr, splitting the operation into
 * multiple calls if necessary.
 *
 * Returns the number of bytes read on success (which will be len if no errors occur),
 * or 0 if an error is encountered.
 */
size_t disk_read(uint32_t addr, void *buffer, const size_t len) {
    size_t total_read = 0;

    /* Get disk info for current disk (curr_disk is assumed global) */
    identifyDeviceData *disk = disks[curr_disk];
    if (!disk || !disk->valid)
        return 0;


    size_t sector_size = disk->logical_sector_size;
    /* Calculate the number of sectors needed to cover len bytes (rounding up) */
    size_t total_sectors = (len + sector_size - 1) / sector_size;

    /* Temporary buffer to hold one sector's data */
    uint8_t temp_buffer[sector_size * MAX_SECTORS_PER_CALL];

    while (total_sectors > 0) {
        uint8_t sectors_this_call;
        if (total_sectors >= MAX_SECTORS_PER_CALL)
            sectors_this_call = 0;
        else
            sectors_this_call = total_sectors;

        /* Read sectors from the disk */
        if (!ata_read_sectors(curr_disk, (uint32_t) addr, sectors_this_call, temp_buffer))
            return total_read;


        /* Determine how many sectors were read in this call */
        size_t sectors_read = (sectors_this_call == 0 ? MAX_SECTORS_PER_CALL : sectors_this_call);
        size_t bytes_this_call = sectors_read * sector_size;

        // Adjust the number of bytes read if necessary
        if (total_read + bytes_this_call > len)
            bytes_this_call = len - total_read;

        memcpy((uint8_t *) buffer + total_read, temp_buffer, bytes_this_call);
        total_read += bytes_this_call;
        total_sectors -= sectors_read;
        addr += sectors_read;
    }

    return total_read;
}


/*
 * Write len bytes to the current disk (assumed to be disks[curr_disk])
 * starting at logical block address lba, splitting the operation into
 * multiple calls if necessary.
 *
 * Returns the number of bytes written on success (which will be len if no errors occur),
 *
 */

size_t disk_write(uint32_t lba, const void *buffer, const size_t len) {
    size_t total_written = 0;

    /* Get disk info for current disk (curr_disk is assumed global) */
    identifyDeviceData *disk = disks[curr_disk];
    if (!disk || !disk->valid)
        return 0;

    size_t sector_size = disk->logical_sector_size;
    size_t total_sectors = len / sector_size;

    /* Temporary buffer to hold one sector's data */
    uint8_t temp_buffer[sector_size * MAX_SECTORS_PER_CALL]; // TODO: Use kmalloc instead of using the stack
    while (total_sectors > 0) {
        uint8_t sectors_this_call = (total_sectors >= MAX_SECTORS_PER_CALL)
                                        ? 0 // ATA: 0→256
                                        : (uint8_t) total_sectors; // ATA: 1..255

        const size_t sectors_xfer = (sectors_this_call == 0) ? MAX_SECTORS_PER_CALL : sectors_this_call;
        const size_t bytes_this_call = sectors_xfer * sector_size;

        memcpy(temp_buffer, (const uint8_t *) buffer + total_written, bytes_this_call);

        if (!ata_write_sectors(curr_disk, lba, sectors_this_call, temp_buffer))
            return total_written;

        total_written += bytes_this_call;
        total_sectors -= sectors_xfer;
        lba += sectors_xfer;
    }


    if (len % disk->logical_sector_size) {
        // The last sector is not full, so we need to read it and write it back to make sure we don't overwrite data
        uint32_t last_sector = lba;
        uint8_t temp[disk->logical_sector_size];
        if (!ata_read_sectors(curr_disk, last_sector, 1, temp))
            return total_written;
        memcpy(temp, (uint8_t *) buffer + total_written, len % disk->logical_sector_size);
        if (!ata_write_sectors(curr_disk, last_sector, 1, temp))
            return total_written;
    }
    return len;
}

size_t disk_get_current_disk_logical_sector_size() {
    return disks[curr_disk]->logical_sector_size;
}

