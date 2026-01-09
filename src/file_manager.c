#include <clip_metadata.h>
#include <file_manager.h>
#include <stack.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool file_init(File_t *f, const char *data_file, const char *index_file) {
	if (!data_file || !index_file) return false;

	f->data_file = fopen(data_file, "rb+");
	f->index_file = fopen(index_file, "rb+");
	if (!(f->data_file) || !(f->index_file)) {
		if (f->data_file) fclose(f->data_file);
		if (f->index_file) fclose(f->index_file);

		f->data_file = fopen(data_file, "wb+");
		f->index_file = fopen(index_file, "wb+");
		if (!(f->data_file) || !(f->index_file)) return false;
	}

	return true;
}

void file_deinit(File_t *f) {
	if (f->data_file) fclose(f->data_file);
	if (f->index_file) fclose(f->index_file);
}

bool write_index(File_t *f, const Index_t *index) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->index_file, 0, SEEK_END);

	uint8_t i = 0;
	while (i < 8) {
		fputc((uint8_t)((index->offset >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	i = 0;
	while (i < 8) {
		fputc((uint8_t)((index->timestamp >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	i = 0;
	while (i < 4) {
		fputc((uint8_t)((index->hash >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	i = 0;
	while (i < 4) {
		fputc((uint8_t)((index->data_size >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	return true;
}

bool read_indexs(File_t *f, Stack **stack) {
	if (stack == NULL) return false;
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->index_file, 0, SEEK_SET);

	int c = 0;
	c = fgetc(f->index_file);
	while (c != EOF) {
		Index_t index;
		index.offset = 0;
		index.hash = 0;
		index.data_size = 0;

		for (uint8_t i = 0; i < 8; i++) {
			if (c == EOF) return false;
			index.offset = index.offset | ((uint64_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		for (uint8_t i = 0; i < 8; i++) {
			if (c == EOF) return false;
			index.timestamp = index.timestamp | ((uint64_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		for (uint8_t i = 0; i < 4; i++) {
			if (c == EOF) return false;
			index.hash = index.hash | ((uint32_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		for (uint8_t i = 0; i < 4; i++) {
			if (c == EOF) return false;
			index.data_size = index.data_size | ((uint32_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		if (index.data_size <= 0 || index.offset < 0 || index.hash <= 0)
			return false;

		bool is_incr;
		if (!is_INCR(f, index.offset, &is_incr)) return false;

		ClipMetadata_t clip;
		clip.data_size = index.data_size;
		clip.offset = index.offset;
		clip.hash = index.hash;
		clip.data = NULL;
		clip.new_clip = false;
		clip.is_INCR = is_incr;

		if (!is_incr) {
			unsigned char *data = malloc(sizeof(char) * index.data_size);
			if (data == NULL) return false;


			if (!read_data(f, data, index.offset, index.data_size)) {
				free(data);
				return false;
			}

			clip.data = data;
			push(stack, &clip);
			free(data);
		} else {
			push(stack, &clip);
		}
	}

	return true;
}

bool init_write_data(File_t *f, uint8_t is_incr, uint64_t *offset) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->data_file, 0, SEEK_END);

	off_t pos = ftello(f->data_file);
	if (pos < 0) return false;
	*offset = (uint64_t)pos;

	fputc((is_incr ? (uint8_t)0x01 : (uint8_t)0x00), f->data_file);

	return true;
}

bool finish_write_data(File_t *f) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->data_file, 0, SEEK_END);
	fputc((uint8_t)0x00, f->data_file);
	fputc((uint8_t)0x00, f->data_file);
	fputc((uint8_t)0x00, f->data_file);
	fputc((uint8_t)0x00, f->data_file);

	return true;
}

bool write_data(File_t *f, const unsigned char *data, uint32_t size) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->data_file, 0, SEEK_END);
	fwrite((uint8_t *)data, sizeof(char), size, f->data_file);
	return true;
}

bool is_INCR(File_t *f, uint64_t offset, bool *is_incr) {
	if (!(f->index_file) || !(f->data_file)) return false;

	if (fseeko(f->data_file, (off_t)offset, SEEK_SET) != 0) return false;

	/* First byte before data */
	int c = fgetc(f->data_file);
	if (c < 0) return false;

	if ((char)c == 0) {
		*is_incr = false;
	} else {
		*is_incr = true;
	}

	return true;
}

bool read_data(File_t *f, unsigned char *data, uint64_t offset, uint32_t size) {
	if (!(f->index_file) || !(f->data_file) || (data == NULL)) return false;

	/* fseeko is UNIX standard. We are using here so that we can use our
	 * uint64_t offset. fseek only supports long type.  */
	if (fseeko(f->data_file, (off_t)offset, SEEK_SET) != 0) return false;

	/* We already checked if this byte in is_INCR. So just skip it.
	 */
	fgetc(f->data_file);

	if (((size_t)size !=
	     fread(data, sizeof(char), (size_t)size, f->data_file)) != 0)
		return false;

	return true;
}
