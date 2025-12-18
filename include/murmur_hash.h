#ifndef MURMUR_HASH_
#define MURMUR_HASH_

#include <stdint.h>

#define C1            0xcc9e2d51
#define C2            0x1b873593
#define R1            15
#define R2            13
#define M             5
#define N             0xe6546b64

#define seed          0x12345678

#define ROL32(num, x) (((num) << (x)) | ((num) >> (32 - (x))))

typedef struct {
	uint32_t hash;
	uint32_t len;
	uint32_t tails_size;
	uint8_t tails[3];
} Murmur_t;

void hash_init(Murmur_t *murmur);

void hash_update(Murmur_t *murmur, const char *key);

void hash_finalize(Murmur_t *murmur);

#endif
