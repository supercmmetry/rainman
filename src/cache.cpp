#include "cache.h"

rainman::cache::cache(FILE *fp, uint64_t size) {
    page_file = fp;
    page_size = size;
    page = new uint8_t[size];
}

uint8_t rainman::cache::get_byte(uint64_t index) {
    auto offset = index / page_size;
    auto page_index = index % page_size;
    if (offset == page_offset) {
        return page[page_index];
    } else {
        // Swap page
        fseek(page_file, page_offset * page_size, SEEK_SET);
        fwrite(page, 1, page_size, page_file);

        fseek(page_file, offset * page_size, SEEK_SET);
        fread(page, 1, page_size, page_file);
        page_offset = offset;
        return page[page_index];
    }
}

uint8_t rainman::cache::set_byte(uint8_t byte, uint64_t index) {
    auto offset = index / page_size;
    auto page_index = index % page_size;
    if (offset == page_offset) {
        page[page_index] = byte;
    } else {
        // Swap page
        fseek(page_file, page_offset * page_size, SEEK_SET);
        fwrite(page, 1, page_size, page_file);

        fseek(page_file, offset * page_size, SEEK_SET);
        fread(page, 1, page_size, page_file);
        page_offset = offset;
        page[page_index] = byte;
    }
}
