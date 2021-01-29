#include "rainman/cache.h"

rainman::cache::_icache::_icache(FILE *fp, uint64_t size, const Allocator &allocator) : _allocator(allocator) {
    page_file = fp;
    page_size = size;
    page = _allocator.rmalloc<uint8_t>(size);
}

uint8_t rainman::cache::_icache::get_byte(uint64_t index) {
    auto offset = index / page_size;
    auto page_index = index % page_size;
    if (offset == page_offset) {
        return page[page_index];
    } else {
        // Swap page
        std::fseek(page_file, page_offset * page_size, SEEK_SET);
        std::fwrite(page, 1, page_size, page_file);

        std::fseek(page_file, offset * page_size, SEEK_SET);
        std::fread(page, 1, page_size, page_file);
        page_offset = offset;
        return page[page_index];
    }
}

void rainman::cache::_icache::set_byte(uint8_t byte, uint64_t index) {
    auto offset = index / page_size;
    auto page_index = index % page_size;
    if (offset == page_offset) {
        page[page_index] = byte;
    } else {
        // Swap page
        std::fseek(page_file, page_offset * page_size, SEEK_SET);
        std::fwrite(page, 1, page_size, page_file);

        std::fseek(page_file, offset * page_size, SEEK_SET);
        std::fread(page, 1, page_size, page_file);
        page_offset = offset;
        page[page_index] = byte;
    }
}

rainman::cache::_icache::_icache(const std::string &filename, uint64_t size, const rainman::Allocator &allocator) {
    std::remove(filename.c_str());
    auto tmp = std::fopen(filename.c_str(), "a");
    std::fclose(tmp);

    page_file = std::fopen(filename.c_str(), "rb+");
    page_size = size;
    page = _allocator.rmalloc<uint8_t>(size);
}

rainman::cache::_icache::~_icache() {
    _allocator.rfree(page);
    std::fclose(page_file);
}

rainman::cache::cache(FILE *fp, uint64_t size, const Allocator &allocator) {
    _allocator = allocator;
    _inner = _allocator.rnew<_icache>(1, fp, size, allocator);
}

rainman::cache::cache(const std::string &filename, uint64_t page_size, const Allocator &allocator) {
    _allocator = allocator;
    _inner = _allocator.rnew<_icache>(1, filename, page_size, allocator);
}

rainman::cache::~cache() {
    if (!refs()) {
        _allocator.rfree(_inner);
    }
}

rainman::cache::cache(rainman::cache &copy) : ReferenceCounter(copy) {
    _inner = copy._inner;
    _allocator = copy._allocator;
}

rainman::cache &rainman::cache::operator=(const rainman::cache &rhs) {
    if (this != &rhs) {
        ReferenceCounter::copy(*this, rhs, true);
        _inner = rhs._inner;
        _allocator = rhs._allocator;
    }

    return *this;
}
