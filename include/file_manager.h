#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <clipqueue.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	FILE *data_file;
	FILE *index_file;
} File_t;

typedef struct {
	uint64_t offset;
	int64_t timestamp;
	uint32_t hash;
	uint32_t data_size;
} Index_t;

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
int64_t timestamp - timestamp at data is copied.
uint32_t hash   - hash of data.
uint32_t   - size of data.


DATA FILE FORMAT
01 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

First byte is for uint8_t is_incr.
Rememaining blocks are for data.
Last 4 blocks are zero which shows end of data.

*/

bool file_init(File_t *f, const char *data_file, const char *index_file);

void file_deinit(File_t *f);

bool write_index(File_t *f, const Index_t *index);

bool read_indexs(File_t *f, ClipQueue **clipqueue);

bool init_write_data(File_t *f, uint8_t is_incr, uint64_t *offset);

bool finish_write_data(File_t *f);

bool write_data(File_t *f, const unsigned char *data, uint32_t size);

bool is_INCR(File_t *f, uint64_t offset, bool *is_incr);

bool read_data(File_t *f, unsigned char *data, uint64_t offset, uint32_t size);

#endif
