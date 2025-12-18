#include <file_manager.h>
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
		f->data_file = fopen(data_file, "wb+");
		f->index_file = fopen(index_file, "wb+");
		if (!(f->data_file) || !(f->index_file)) false;
	}

	return true;
}

void file_deinit(File_t *f) {
	if (f->data_file) fclose(f->data_file);
	if (f->index_file) fclose(f->index_file);
}

void write_index(File_t *f, uint64_t offset, uint32_t hash, unsigned long data_size) {
	fseek(f->index_file, 0, SEEK_END);

    uint8_t i = 0;

    while(i < 8){
        fputc((uint8_t)((offset >> i * 8 ) & 0xFF), f->index_file);
        i++;
    }
    
    i = 0;
	while (i < 4) {
		fputc((uint8_t)((hash >> i * 8) & 0xFF), f->index_file);
        i++;
	}

    i = 0;
	while (i < 4) {
		fputc((uint8_t)((data_size >> i * 8 ) & 0xFF), f->index_file);
        i++;
	}
}

int main() {
	File_t file;
	if (!file_init(&file, "clipboard", "index")) return 1;

	file_deinit(&file);
}
