#ifndef RAINMAN_CACHE_H
#define RAINMAN_CACHE_H

#include <cstdio>
#include <cstdint>
#include <mutex>

namespace rainman {
    class cache {
    private:
        FILE *page_file;
        uint64_t page_size{};
        uint64_t page_offset{};
        uint8_t *page{};
        std::mutex mutex;

        uint8_t get_byte(uint64_t index);

        uint8_t set_byte(uint8_t byte, uint64_t index);

    public:
        cache(FILE *fp, uint64_t size);

        template<typename T>
        T read(uint64_t index) {
            uint8_t data[sizeof(T)];
            for (uint64_t i = 0; i < sizeof(T); i++) {
                data[i] = get_byte(index + i);
            }

            return T(data);
        }

        template<typename T>
        void write(T obj, uint64_t index) {
            auto data = (uint8_t *) &obj;
            for (uint64_t i = 0; i < sizeof(T); i++) {
                set_byte(data[i], index + i);
            }
        }
    };
}

#endif
