#include "cache.h"

rainman::cache::cache(FILE *fp, uint64_t size, const Allocator &allocator) : _allocator(allocator) {
    page_file = fp;
    page_size = size;
    page = _allocator.rmalloc<uint8_t>(size);
    mutex = new std::mutex;
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

void rainman::cache::set_byte(uint8_t byte, uint64_t index) {
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

rainman::cache::cache(const rainman::cache &copy) : ReferenceCounter(copy), _allocator(copy._allocator) {
    copy.mutex->lock();
    page_file = copy.page_file;
    page_size = copy.page_size;
    page = copy.page;
    page_offset = copy.page_offset;
    mutex = copy.mutex;
    eof = copy.eof;
    lenmap = copy.lenmap;
    fragments = copy.fragments;
    copy.mutex->unlock();
}

rainman::cache &rainman::cache::operator=(const rainman::cache &rhs) {
    if (this != &rhs) {
        rhs.mutex->lock();
        ReferenceCounter::copy(*this, rhs, true);
        page_file = rhs.page_file;
        page_size = rhs.page_size;
        page = rhs.page;
        page_offset = rhs.page_offset;
        mutex = rhs.mutex;
        eof = rhs.eof;
        lenmap = rhs.lenmap;
        fragments = rhs.fragments;
        _allocator = rhs._allocator;
        rhs.mutex->unlock();
    }

    return *this;
}
