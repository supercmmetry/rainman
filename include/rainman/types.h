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
 * Also includes some wrappers for making things easier.
 */

namespace rainman {
    template<class Type>
    class ptr : private ReferenceCounter {
    private:
        Type *_inner;
        uint64_t _n{};
        uint64_t _offset{};
        Allocator _allocator{};

    public:
        ptr(const ptr<Type> &copy) : ReferenceCounter(copy), _allocator(copy._allocator) {
            _inner = copy._inner;
            _n = copy._n;
            _offset = copy._offset;
        }

        explicit ptr(ptr<Type> *copy) : ReferenceCounter(*copy), _allocator(copy->_allocator) {
            _inner = copy->_inner;
            _n = copy->_n;
            _offset = copy->_offset;
        }

        ptr(Type *inner, uint64_t n_elems, const Allocator &allocator = Allocator()) : _allocator(allocator) {
            _inner = inner;
            _n = n_elems;
        }

        template <typename ...Args>
        ptr(const Allocator& allocator, uint64_t n_elems, Args ...args) : _allocator(allocator) {
            _inner = _allocator.rnew<Type>(n_elems, std::forward<Args>(args)...);
            _n = n_elems;
        }

        ptr(const Allocator& allocator) : _allocator(allocator) {
            _inner = _allocator.rnew<Type>(1);
            _n = 1;
        }

        template <typename ...Args>
        ptr(uint64_t n_elems, Args ...args) {
            _inner = _allocator.rnew<Type>(n_elems, std::forward<Args>(args)...);
            _n = n_elems;
        }

        ptr() {
            _inner = _allocator.rnew<Type>(1);
            _n = 1;
        }

        ptr<Type> &operator=(const ptr<Type> &rhs) {
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

        inline ptr<Type> &operator=(const Type &rhs) const {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            *(_inner + _offset) = rhs;
            return *this;
        }

        inline Type &operator[](uint64_t index) const {
            auto idx = index + _offset;
            if (idx >= _n || idx < 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return (_inner + _offset)[index];
        }

        inline Type &operator*() const {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return *(_inner + _offset);
        }

        inline Type *operator->() const {
            if (_n == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return _inner + _offset;
        }

        inline Type *pointer() const {
            return _inner + _offset;
        }

        inline Type *inner() const {
            return _inner;
        }

        inline void operator++() {
            if (_offset == _n - 1) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset++;
        }

        inline void operator++(int) {
            if (_offset == _n - 1) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset++;
        }

        inline void operator--() {
            if (_offset == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset--;
        }

        inline void operator--(int) {
            if (_offset == 0) {
                throw MemoryErrors::SegmentationFaultException();
            }

            _offset--;
        }

        inline void operator+=(int64_t x) {
            uint64_t idx;

            if (x >= 0) {
                idx = _offset + (uint64_t)x;
                if (idx >= _n) {
                    throw MemoryErrors::SegmentationFaultException();
                }
            } else {
                if (_offset < (uint64_t)(-x)) {
                    throw MemoryErrors::SegmentationFaultException();
                }

                idx = _offset - (uint64_t)(-x);
            }

            _offset = idx;
        }

        inline void operator-=(int64_t x) {
            operator+=(-x);
        }

        [[nodiscard]] inline uint64_t size() const {
            return _n;
        }

        ~ptr() {
            if (!refs()) {
                _allocator.rfree(_inner);
            }
        }
    };

    template <typename Type>
    using ptr2d = ptr<ptr<Type>>;

    template <typename Type, typename ...Args>
    ptr2d<Type> make_ptr2d(uint64_t rows, uint64_t cols, Args ...args) {
        auto colptr = ptr<Type>(cols, std::forward<Args>(args)...);
        return ptr2d<Type>(rows, &colptr);
    }

    template <typename Type>
    using ptr3d = ptr<ptr2d<Type>>;

    template <typename Type, typename ...Args>
    ptr3d<Type> make_ptr3d(uint64_t depth, uint64_t rows, uint64_t cols, Args ...args) {
        auto inner_ptr = make_ptr2d<Type>(rows, cols, std::forward<Args>(args)...);
        return ptr3d<Type>(depth, &inner_ptr);
    }

    /*
     * virtual_array takes a rainman::cache and maps an array to it.
     * The subscripting operator can only be used for reading purposes.
     * For writing to an index use set().
     */
    template<class Type>
    class virtual_array : private ReferenceCounter {
    private:
        cache _cache;
        uint64_t _index{};
        uint64_t _n{};
    public:
        virtual_array(const cache &cache, uint64_t n) {
            this->_cache = cache;
            _index = _cache.allocate<Type>(n);
            _n = n;
        }

        virtual_array(const virtual_array &copy) : ReferenceCounter(copy) {
            _cache = copy._cache;
            _index = copy._index;
        }

        virtual_array &operator=(const virtual_array &rhs) {
            if (this != &rhs) {
                ReferenceCounter::copy(*this, rhs, true);
                _cache = rhs._cache;
                _index = rhs._index;
            }

            return *this;
        }

        Type operator[](uint64_t i) {
            return _cache.read<Type>(_index + sizeof(Type) * i);
        }

        void set(Type obj, uint64_t i) {
            _cache.write(obj, _index + sizeof(Type) * i);
        }

        [[nodiscard]] uint64_t size() const {
            return _n;
        }

        ~virtual_array() {
            if (!refs()) {
                _cache.deallocate(_index);
            }
        }
    };

    /*
     * result is a wrapper for checking whether the received inner-type is valid or not.
     */

    template<typename LType, typename RType = std::string>
    class result {
    private:
        enum ResultType {
            Ok,
            Error
        };

        ResultType _type = Error;
        LType _inner;
        RType _err;
    public:
        result() = default;

        static result ok(const LType &inner) {
            auto v = result();
            v._type = Ok;
            v._inner = inner;

            return v;
        }

        static result err(const RType &err) {
            auto v = result();
            v._type = Error;
            v._err = err;

            return v;
        }

        result(const result &copy) {
            _inner = copy._inner;
            _err = copy._err;
            _type = copy._type;
        }

        result &operator=(const result &rhs) {
            if (this != &rhs) {
                _inner = rhs._inner;
                _err = rhs._err;
                _type = rhs._type;
            }

            return *this;
        }

        [[nodiscard]] bool is_ok() const {
            return _type == Ok;
        }

        [[nodiscard]] bool is_err() const {
            return _type == Error;
        }

        LType &inner() {
            return _inner;
        }

        RType &err() {
            return _err;
        }
    };

    /*
     * option is a wrapper for checking whether the received inner-type exists or not.
     */
    template<typename Type>
    class option {
    private:
        enum OptionType {
            Some,
            None
        };

        OptionType _type = None;
        Type _inner;
    public:
        option() = default;

        option(const Type &inner) {
            _inner = inner;
            _type = Some;
        }

        option(const option<Type> &copy) {
            _inner = copy._inner;
            _type = copy._type;
        }

        option<Type> &operator=(const option<Type> &rhs) {
            if (this != &rhs) {
                _inner = rhs._inner;
                _type = rhs._type;
            }

            return *this;
        }

        option<Type> &operator=(const Type &rhs) {
            _inner = rhs;
            _type = Some;

            return *this;
        }

        [[nodiscard]] bool is_some() const {
            return _type == Some;
        }

        [[nodiscard]] bool is_none() const {
            return _type == None;
        }

        Type &inner() {
            return _inner;
        }
    };
}

#endif