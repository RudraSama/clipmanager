#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <stack.h>
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




DATA FILE FORMAT
01 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

First byte is for uint8_t copy_to_memory.
Rememaining blocks are for data.
Last 4 blocks are zero which shows end of data.

*/

bool file_init(File_t *f, const char *data_file, const char *index_file);

void file_deinit(File_t *f);

bool write_index(File_t *f, uint64_t offset, uint32_t hash,
                 unsigned long data_size);

bool read_indexs(File_t *f, Stack **stack);

bool init_write_data(File_t *f, uint8_t copy_to_memory, uint64_t *offset);

bool finish_write_data(File_t *f);

bool write_data(File_t *f, const unsigned char *data, uint32_t size);

bool get_copy_to_memory(File_t *f, uint64_t offset);

bool read_data(File_t *f, unsigned char *data, uint64_t offset, uint32_t size,
               uint8_t *copy_to_memory);

#endif
