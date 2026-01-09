#ifndef CLIP_METADATA_H
#define CLIP_METADATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	unsigned char *data;
	uint64_t offset;
	int64_t timestamp;
	uint32_t hash;
	size_t data_size;
	bool is_INCR;
	bool new_clip;
} ClipMetadata_t;

#endif
