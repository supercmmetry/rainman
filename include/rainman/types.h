#ifndef RAINMAN_TYPES_H
#define RAINMAN_TYPES_H

#include "memmgr.h"
#include "global.h"
#include "errors.h"
#include "cache.h"
#include "utils.h"

/*
 * Context-less wrappers for memory allocation. Uses the rainman global memory manager by default.
 * These types are not thread-safe.
 */

namespace rainman {
    template<class T>
    class ptr : private ReferenceCounter {
    private:
        T *_inner;
        uint64_t _n{};
        uint64_t _offset{};
        Allocator _allocator{};

    public:
        ptr(const Allocator &allocator = Allocator()): _allocator(allocator) {
            _inner = _allocator.rnew<T>();
            _n = 1;
        }

        ptr(uint64_t n_elems, const Allocator &allocator = Allocator()): _allocator(allocator) {
            _inner = _allocator.rmalloc<T>(n_elems);
            _n = n_elems;
        }

        ptr(T *inner, uint64_t n_elems, const Allocator &allocator = Allocator()) : _allocator(allocator) {
            _inner = inner;
            _n = n_elems;
        }

        ptr(ptr<T> &copy) : ReferenceCounter(copy), _allocator(copy._allocator) {
            _inner = copy._inner;
            _n = copy._n;
            _offset = copy._offset;
        }

        ptr<T> &operator=(const ptr<T> &rhs) {
            if (this != &rhs) {
                ReferenceCounter::copy(*this, rhs, true);
                _allocator.rfree(_inner);
                _inner = rhs._inner;
                _n = rhs._n;
                _offset = rhs._offset;
                _allocator = rhs._allocator;
            }

            return *this;
        }

        ptr<T> &operator=(const T &rhs) {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            *(_inner + _offset) = rhs;
        }

        T &operator[](uint64_t index) {
            auto idx = index + _offset;
            if (idx >= _n || idx < 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return (_inner + _offset)[index];
        }

        T &operator*() {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return *(_inner + _offset);
        }

        T *operator->() {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return _inner + _offset;
        }

        ptr &operator++() {
            if (_offset == _n - 1) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset++;
            return *this;
        }

        ptr &operator--() {
            if (_offset == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset--;
            return *this;
        }

        uint64_t length() {
            return _n;
        }

        ~ptr() {
            if (!refs()) {
                _allocator.rfree(_inner);
            }
        }
    };


    /*
     * virtual_array takes a rainman::cache and maps an array to it.
     * The subscripting operator can only be used for reading purposes.
     * For writing to an index use set().
     */
    template<class T>
    class virtual_array : public ReferenceCounter {
    private:
        cache _cache{};
        uint64_t _index{};
        uint64_t _n{};
    public:
        virtual_array(const cache &cache, uint64_t n) {
            this->_cache = cache;
            _index = _cache.allocate<T>(n);
            _n = n;
        }

        virtual_array(const virtual_array &copy) : ReferenceCounter(copy) {
            _cache = copy._cache;
            _index = copy._index;
        }

        virtual_array &operator=(const virtual_array &rhs) {
            if (this != &rhs) {
                ref_copy(rhs, true);
                _cache = rhs._cache;
                _index = rhs._index;
            }

            return *this;
        }

        T operator[](uint64_t i) {
            return _cache.read<T>(_index + sizeof(T) * i);
        }

        void set(T obj, uint64_t i) {
            _cache.write(obj, _index + sizeof(T) * i);
        }

        uint64_t length() {
            return _n;
        }

        ~virtual_array() {
            if (!refs()) {
                _cache.deallocate(_index);
            }
        }
    };
}

#endif