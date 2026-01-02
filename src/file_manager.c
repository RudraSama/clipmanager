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

bool write_index(File_t *f, uint64_t offset, uint32_t hash,
                 unsigned long data_size) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->index_file, 0, SEEK_END);

	uint8_t i = 0;
	while (i < 8) {
		fputc((uint8_t)((offset >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	i = 0;
	while (i < 4) {
		fputc((uint8_t)((hash >> i * 8) & 0xFF), f->index_file);
		i++;
	}

	i = 0;
	while (i < 4) {
		fputc((uint8_t)((data_size >> i * 8) & 0xFF), f->index_file);
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
		uint64_t offset = 0;
		uint32_t hash = 0;
		unsigned long data_size = 0;

		for (uint8_t i = 0; i < 8; i++) {
			if (c == EOF) return false;
			offset = offset | ((uint64_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		for (uint8_t i = 0; i < 4; i++) {
			if (c == EOF) return false;
			hash = hash | ((uint32_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		for (uint8_t i = 0; i < 4; i++) {
			if (c == EOF) return false;
			data_size = data_size | ((uint32_t)(uint8_t)c << i * 8);
			c = fgetc(f->index_file);
		}

		if (data_size <= 0 || offset < 0 || hash <= 0) return false;

		unsigned char *data = malloc(sizeof(char) * data_size);
		if (data == NULL) return false;

		uint8_t copy_to_memory = 0;

		if (!read_data(f, data, offset, data_size, &copy_to_memory)) {
			free(data);
			return false;
		}

		if (copy_to_memory == 0) {
			push(stack, "0", offset, hash, data_size);
		} else {
			push(stack, data, offset, hash, data_size);
		}

		free(data);
	}

	return true;
}

bool init_write_data(File_t *f, uint8_t copy_to_memory, uint64_t *offset) {
	if (!(f->index_file) || !(f->data_file)) return false;

	fseek(f->data_file, 0, SEEK_END);

	off_t pos = ftello(f->data_file);
	if (pos < 0) return false;
	*offset = (uint64_t)pos;

	fputc((copy_to_memory ? (uint8_t)0x01 : (uint8_t)0x00), f->data_file);

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

bool get_copy_to_memory(File_t *f, uint64_t offset, uint8_t copy_to_memory) {
	if (!(f->index_file) || !(f->data_file) || (data == NULL)) return false;

	if (fseeko(f->data_file, (off_t)offset, SEEK_SET) != 0) return false;

	/* First byte before data */
	int c = fgetc(f->data_file);
	if (c < 0) return false;

	if ((char)c == 0) {
		*copy_to_memory = 0;
	} else {
		*copy_to_memory = 1;
	}


	return true;
}

bool read_data(File_t *f, unsigned char *data, uint64_t offset, uint32_t size,
               uint8_t *copy_to_memory) {
	if (!(f->index_file) || !(f->data_file) || (data == NULL)) return false;

	/* fseeko is UNIX standard. We are using here so that we can use our
	 * uint64_t offset. fseek only supports long type.  */
	if (fseeko(f->data_file, (off_t)offset, SEEK_SET) != 0) return false;

	/* Check if data should be copied to memory or not. */
	int c = fgetc(f->data_file);
	if (c < 0) return false;

	if ((char)c == 0) {
		/* We have to copy something for preview but not doing right now. */
		*copy_to_memory = 0;
		return true;
	}

	*copy_to_memory = 1;

	if ((size_t)size != fread(data, sizeof(char), (size_t)size, f->data_file) !=
	    0)
		return false;

	return true;
}

int main() {
	File_t f;
	Stack *stack = stack_init();
	if (stack == NULL) return 0;

	if (!file_init(&f, "clipboard", "index")) return false;

	const unsigned char *d = "HELLO HOW ARE U ??";

	uint64_t offset = 0;
	init_write_data(&f, 1, &offset);
	write_data(&f, d, strlen(d));
	finish_write_data(&f);

	write_index(&f, offset, 0x92384, strlen(d));

	uint8_t copy_to_memory = 0;
	read_indexs(&f, &stack);

	file_deinit(&f);
	stack_deinit(&stack);
	return 0;
}
