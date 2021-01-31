#ifndef RAINMAN_CACHE_H
#define RAINMAN_CACHE_H

#include <cstdio>
#include <cstdint>
#include <mutex>
#include <vector>
#include <unordered_map>
#include "utils.h"

namespace rainman {
    class cache : private ReferenceCounter {
    private:
        class _icache {
        private:
            struct cache_fragment {
                uint64_t index;
                uint64_t length;
            };

            FILE *page_file{};
            uint64_t page_size{};
            uint64_t page_offset{};
            uint8_t *page{};
            uint64_t eof{};
            std::mutex mutex{};

            std::vector<cache_fragment> fragments;
            std::unordered_map<uint64_t, uint64_t> lenmap;
            Allocator _allocator{};

            uint8_t get_byte(uint64_t index);

            void set_byte(uint8_t byte, uint64_t index);

        public:
            _icache() = default;

            _icache(FILE *fp, uint64_t size, const Allocator &allocator = Allocator());

            _icache(const std::string &filename, uint64_t size, const Allocator &allocator = Allocator());

            template<typename T>
            uint64_t allocate(uint64_t n) {
                uint64_t size = n * sizeof(T);

                mutex.lock();

                bool found_fragment = false;
                uint64_t alloc_index = 0;

                // First-fit
                for (uint64_t i = 0; i < fragments.size(); i++) {
                    auto len = fragments[i].length;
                    if (len > size) {
                        alloc_index = fragments[i].index;
                        found_fragment = true;
                        fragments[i].length -= size;
                        fragments[i].index += size;
                        break;
                    } else if (len == size) {
                        alloc_index = fragments[i].index;
                        found_fragment = true;
                        fragments.erase(fragments.begin() + i);
                        break;
                    }
                }

                if (!found_fragment) {
                    alloc_index = eof;
                    eof += size;
                }

                lenmap[alloc_index] = size;
                mutex.unlock();

                return alloc_index;
            }

            void deallocate(uint64_t index) {
                mutex.lock();
                fragments.push_back(cache_fragment{
                        .index=index,
                        .length=lenmap[index]
                });

                lenmap.erase(index);
                mutex.unlock();
            }

            // Read an object from the cache at a byte-index.
            // Note: This only works for primitives and 1-byte packed structs.
            template<typename T>
            T read(uint64_t index) {
                mutex.lock();
                uint8_t data[sizeof(T)];
                for (uint64_t i = 0; i < sizeof(T); i++) {
                    data[i] = get_byte(index + i);
                }

                mutex.unlock();
                return *reinterpret_cast<T *>(data);
            }

            // Write an object to the cache at a byte-index.
            // Note: This only works for primitives and 1-byte packed structs.
            template<typename T>
            void write(T obj, uint64_t index) {
                mutex.lock();
                auto data = reinterpret_cast<uint8_t *>(&obj);
                for (uint64_t i = 0; i < sizeof(T); i++) {
                    set_byte(data[i], index + i);
                }
                mutex.unlock();
            }

            ~_icache();
        };

        _icache *_inner{};
        Allocator _allocator;
    public:
        cache() = default;

        cache(FILE *fp, uint64_t size, const Allocator &allocator = Allocator());

        cache(const std::string &filename, uint64_t page_size, const Allocator &allocator = Allocator());

        cache(cache &copy);

        cache &operator=(const cache &rhs);

        template<typename Type>
        uint64_t allocate(uint64_t n) {
            return _inner->template allocate<Type>(n);
        }

        void deallocate(uint64_t index) {
            _inner->deallocate(index);
        }

        // Read an object from the cache at a byte-index.
        // Note: This only works for primitives and 1-byte packed structs.
        template<typename Type>
        Type read(uint64_t index) {
            return _inner->template read<Type>(index);
        }

        // Write an object to the cache at a byte-index.
        // Note: This only works for primitives and 1-byte packed structs.
        template<typename Type>
        void write(Type obj, uint64_t index) {
            _inner->template write<Type>(obj, index);
        }

        ~cache();
    };

}

#endif
