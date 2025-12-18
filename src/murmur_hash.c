#include <murmur_hash.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hash_init(Murmur_t *murmur) {
	murmur->hash = seed;
	murmur->len = 0;
	murmur->tails_size = 0;
	murmur->tails[0] = 0;
	murmur->tails[1] = 0;
	murmur->tails[2] = 0;
}

void hash_update(Murmur_t *murmur, const char *key) {
	uint32_t len = strlen(key);
	uint32_t i = 0;
	uint32_t k = 0;

	murmur->len += len;

	switch (murmur->tails_size & 3) {
		case 3: {
			k = (uint32_t)(uint8_t)murmur->tails[0] |
			    (uint32_t)(uint8_t)murmur->tails[1] << 8 |
			    (uint32_t)(uint8_t)murmur->tails[2] << 16 |
			    (uint32_t)(uint8_t)key[i] << 24;
			i++;
			break;
		}
		case 2: {
			k = (uint32_t)(uint8_t)murmur->tails[0] |
			    (uint32_t)(uint8_t)murmur->tails[1] << 8 |
			    (uint32_t)(uint8_t)key[i] << 16 |
			    (uint32_t)(uint8_t)key[i + 1] << 24;
			i += 2;
			break;
		}
		case 1: {
			k = (uint32_t)(uint8_t)murmur->tails[0] |
			    (uint32_t)(uint8_t)key[i] << 8 |
			    (uint32_t)(uint8_t)key[i + 1] << 16 |
			    (uint32_t)(uint8_t)key[i + 2] << 24;
			i += 3;
			break;
		}
		default:
			break;
	}

	if (murmur->tails_size) {
		murmur->tails_size = 0;

		k = k * (uint32_t)C1;
		k = ROL32(k, R1);
		k = k * (uint32_t)C2;

		murmur->hash ^= k;
		murmur->hash = ROL32(murmur->hash, R2);
		murmur->hash = (murmur->hash * (uint32_t)M) + (uint32_t)N;
	}

	while (i + 4 <= len) {
		k = (uint32_t)(uint8_t)key[i] | (uint32_t)(uint8_t)key[i + 1] << 8 |
		    (uint32_t)(uint8_t)key[i + 2] << 16 |
		    (uint32_t)(uint8_t)key[i + 3] << 24;

		k = k * (uint32_t)C1;
		k = ROL32(k, R1);
		k = k * (uint32_t)C2;

		murmur->hash ^= k;
		murmur->hash = ROL32(murmur->hash, R2);
		murmur->hash = (murmur->hash * (uint32_t)M) + (uint32_t)N;
		i += 4;
	}

	switch ((len - i) & 3) {
		case 3:
			murmur->tails[2] = key[i + 2];
		case 2:
			murmur->tails[1] = key[i + 1];
		case 1:
			murmur->tails[0] = key[i];
			murmur->tails_size = len - i;
	}
}

void hash_finalize(Murmur_t *murmur) {
	uint32_t k = 0;
	switch (murmur->tails_size & 3) {
		case 3:
			k ^= (uint32_t)(uint8_t)murmur->tails[2] << 16;
		case 2:
			k ^= (uint32_t)(uint8_t)murmur->tails[1] << 8;
		case 1:
			k ^= (uint32_t)(uint8_t)murmur->tails[0];
			k = k * (uint32_t)C1;
			k = ROL32(k, R1);
			k = k * (uint32_t)C2;
			murmur->hash = murmur->hash ^ k;
	}

	murmur->hash ^= murmur->len;
	murmur->hash ^= murmur->hash >> 16;
	murmur->hash *= (uint32_t)0x85ebca6b;
	murmur->hash ^= murmur->hash >> 13;
	murmur->hash *= (uint32_t)0xc2b2ae35;
	murmur->hash ^= murmur->hash >> 16;
}
