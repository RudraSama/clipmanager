#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	FILE *data_file;
	FILE *index_file;
} File_t;

/*
INDEX FILE FORMAT
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

First 8 blocks are for uint64_t offset (Little Endian)
Next 4 blocks are for uint32_t hash (Little Endian).
Next 4 blocks are for unsigned long data_size (Little Endian).

uint64_t offset - offset of data in data_file.
uint32_t hash   - hash of data.
unsigned long   - size of data.

*/

bool file_init(File_t *f, const char *data_file, const char *index_file);

void file_deinit(File_t *f);

void write_index(File_t *f, uint64_t offset, uint32_t hash, unsigned long data_size);

void read_indexs(File_t *f);

#endif
