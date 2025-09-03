//
// Created by Yoav on 8/30/2025.
//

#include "fat32.h"
#include "../std/string.h"
#include "../drivers/disk.h"

#define EOC_MIN 0x0FFFFFF8 //end of chain min
#define BAD_CLUSTER 0x0FFFFFF7 //bad cluster
#define FREE_CLUSTER 0x00000000 //free cluster

inline bool is_eoc(const uint32_t entry) {
    return entry > EOC_MIN;
}

inline bool is_bad_cluster(const uint32_t entry) {
    return entry == BAD_CLUSTER;
}

inline uint32_t read_fat_table_entry(uint32_t entry) {
    entry &= 0x0FFFFFFF; // Only the 28 bits are used for the cluster number
    // todo read from the disk the fat table entry for the cluster
    return 0;
}

inline void write_fat_table_entry(uint32_t entry, uint32_t value) {
    entry &= 0x0FFFFFFF; // Only the 28 bits are used for the cluster number

    // todo write to the disk the fat table entry for the cluster
}

/*
 * Convert a regular string to a FAT32 8.3 filename structure.
 * If the extension is missing int the name it will be get txt extension
 */
bool create_fat32_file_name(const char *name, fat32_file_name *out_name) {
    if (name == NULL || out_name == NULL)
        return false;
    uint32_t name_len = strlen(name);
    if (name_len == 0 || name_len > FILE_NAME_LENGTH + FILE_NAME_EXTENSION_LENGTH + 1) // +1 for the dot
        return false;
    int i;
    bool found_dot = false;
    for (i = 0; i < FILE_NAME_LENGTH; ++i) {
        if (name[i] == '.') {
            found_dot = true;
            break;
        }
        out_name->file_name[i] = name[i];
    }
    if (!found_dot && name_len > FILE_NAME_LENGTH) // if the name part is longer than FILE_NAME_LENGTH
        return false;
    if (!found_dot) {
        // pad the name with spaces
        for (int j = i; j < FILE_NAME_LENGTH; j++)
            out_name->file_name[j] = NAME_PAD_CHAR;
        // set default extension to "txt"
        out_name->file_extension[0] = 't';
        out_name->file_extension[1] = 'x';
        out_name->file_extension[2] = 't';
    } else {
        // pad the name with spaces
        for (int j = i; j < FILE_NAME_LENGTH; j++)
            out_name->file_name[j] = NAME_PAD_CHAR;
        i++;
        int ext_i = 0;
        while (i < name_len && ext_i < FILE_NAME_EXTENSION_LENGTH) {
            out_name->file_extension[ext_i] = name[i];
            i++;
            ext_i++;
        }
        // pad the extension with spaces
        for (int j = ext_i; j < FILE_NAME_EXTENSION_LENGTH; j++)
            out_name->file_extension[j] = NAME_PAD_CHAR;
    }
    return true;
}


fat32_time_at_date get_current_time_at_date() {
    //todo implement
    return 0;
}

fat32_date get_current_date() {
    //todo implement
    return 0;
}


void free_chain(uint32_t cluster) {
    //todo implement
}

inline uint32_t get_next_cluster(uint32_t cluster) {
    return read_fat_table_entry(cluster) & 0x0FFFFFFF; // Only the 28 bits are used for the cluster number
}

uint32_t allocate_cluster() {
}




