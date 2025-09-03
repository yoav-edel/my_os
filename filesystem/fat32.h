//
// Created by Yoav on 8/30/2025.
//
#pragma once
#include "../std/stdint.h"
#include "../std/stdbool.h"

#ifndef MYKERNEL_FAT32_H
#define MYKERNEL_FAT32_H

#define FILE_NAME_LENGTH 8
#define FILE_NAME_EXTENSION_LENGTH 3
#define NAME_PAD_CHAR ' '

typedef struct {
} fat32_file;

typedef struct {
}
fat32_directory;


#define  FILE_READONLY 0x01
#define  FILE_HIDDEN 0x02
#define  FILE_SYSTEM 0x04
#define  FILE_VOLUME_ID 0x08
#define  FILE_DIRECTORY 0x10
#define  FILE_ARCHIVE 0x20
typedef uint8_t attributes;

/* 8.3 filename structure
 * Max 8 characters for the name, if less than 8 characters it is padded with spaces
 * Max 3 characters for the extension, if less than 3 characters it is padded with spaces
 * The result will look like this - "name"."ext"
 */

typedef struct {
    char file_name[FILE_NAME_LENGTH];
    char file_extension[FILE_NAME_EXTENSION_LENGTH];
}
fat32_file_name;

//represents the time at a date - 5 bits for the hour (0-23), 6 bits for the minute (0-59),
//5 bits for the second (0-29, in 2-second increments)
typedef uint16_t fat32_time_at_date;

// represent a date - 7 bits for the year (0 = 1980, 127 = 2107), 4 bits for the month (1-12), 5 bits for the day (1-31)
typedef uint16_t fat32_date;

typedef struct {
    fat32_file_name name;
    attributes flags;
    uint8_t reserved;
    uint8_t creation_time_hundredths;
    fat32_time_at_date creation_time_at_date;
    fat32_date creation_date;
    fat32_date last_access_date;
    uint16_t first_cluster_high; // high 2 bytes of the first cluster number
    fat32_time_at_date last_modification_time;
    fat32_date last_modification_date;
    uint16_t first_cluster_low; // low 2 bytes of the first cluster number
    uint32_t file_size; // in bytes
} fat32_file_entry;

bool fat32_create_file(const char *path, fat32_file_name, attributes flags);

bool fat32_create_directory(const char *path, fat32_file_name name);

fat32_file *fat32_open(const char *path, const char *name, attributes flags);

bool fat32_close(fat32_file *file);

uint32_t fat32_read(fat32_file *file, void *buffer, uint32_t size);

uint32_t fat32_write(fat32_file *file, const void *buffer, uint32_t size);

// todo implement these functions

/*
 * bool fat32_delete(const char* path, const char* name);
 * bool fat32_rename(const char *path, const char* new_name);
 * bool fat32_move(const char* old_path, const char* new_path);
 */


#endif //MYKERNEL_FAT32_H
