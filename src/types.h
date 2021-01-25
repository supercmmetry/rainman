#include <mutex>
#include "memmgr.h"
#include "global.h"
#include "errors.h"
#include "cache.h"

/*
 * Context-less wrappers for memory allocation. Uses the rainman global memory manager.
 * These types are not thread-safe.
 */

namespace rainman {

    class ReferenceCounter {
    private:
        uint64_t *_refs{};
        std::mutex *_mutex{};

        void _destroy() {
            _mutex->lock();

            if (*_refs == 0) {
                delete _refs;
                delete _mutex;
            } else {
                *_refs = *_refs - 1;
                _mutex->unlock();
            }
        }

    protected:
        void increment() {
            _mutex->lock();
            *_refs = *_refs + 1;
            _mutex->unlock();
        }

        void decrement() {
            _destroy();
        }

        uint64_t refs() {
            _mutex->lock();
            auto val = *_refs;
            _mutex->unlock();
            return val;
        }

        void copy_ref(const ReferenceCounter &copy, bool destroy = false) {
            if (this != &copy) {
                copy._mutex->lock();
                if (destroy) {
                    _destroy();
                }
                _refs = copy._refs;
                _mutex = copy._mutex;
                *_refs = *_refs + 1;
                copy._mutex->unlock();
            }
        }

    public:
        ReferenceCounter() {
            _refs = new uint64_t;
            _mutex = new std::mutex;
            *_refs = 0;
        }

        ReferenceCounter(const ReferenceCounter &copy) {
            copy_ref(copy);
        }

        ~ReferenceCounter() {
            _destroy();
        }
    };

    template<class T>
    class ptr : public ReferenceCounter {
    private:
        T *_inner;
        uint64_t _n{};
        memmgr *_mgr;
    public:
        ptr(uint64_t n_elems, memmgr *mgr = &rglobalmgr) : _mgr(mgr) {
            _inner = _mgr->r_malloc<T>(n_elems);
            _n = n_elems;
        }

        ptr(T *inner, uint64_t n_elems, memmgr *mgr = &rglobalmgr) : _mgr(mgr) {
            _inner = inner;
            _n = n_elems;
        }

        ptr(ptr<T> &copy) : ReferenceCounter(copy) {
            _inner = copy._inner;
            _n = copy._n;
            _mgr = copy._mgr;
        }

        ptr<T> &operator=(const ptr<T> &rhs) {
            if (this != &rhs) {
                copy_ref(rhs, true);
                _mgr->r_free(_inner);
                _inner = rhs._inner;
                _n = rhs._n;
                _mgr = rhs._mgr;
            }

            return *this;
        }

        T &operator[](uint64_t index) {
            if (index >= _n) {
                throw MemoryErrors::SegmentationFaultException();
            }

            return _inner[index];
        }

        ~ptr() {
            if (refs() == 0) {
                _mgr->r_free(_inner);
            }
        }
    };


    /*
     * virtual_array only supports primitives and 1-byte packed structs.
     * Using other types can introduce undefined behaviour. The subscripting operator
     * can only be used for reading purposes. For writing to an index use set().
     */
    template<class T>
    class virtual_array {
    private:
        cache *_cache{};
        uint64_t _index{};
    public:
        virtual_array(cache *cache, uint64_t n) {
            this->_cache = cache;
            _index = _cache->allocate<T>(n);
        }

        T operator[](uint64_t i) {
            return _cache->read<T>(_index + sizeof(T) * i);
        }

        void set(T obj, uint64_t i) {
            _cache->write(obj, _index + sizeof(T) * i);
        }

        ~virtual_array() {
            _cache->deallocate(_index);
        }
    };
}