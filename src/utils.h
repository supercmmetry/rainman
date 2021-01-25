#ifndef RAINMAN_UTILS_H
#define RAINMAN_UTILS_H

#include <cstdint>
#include <mutex>
#include "global.h"

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
        uint64_t refs() {
            _mutex->lock();
            auto val = *_refs;
            _mutex->unlock();
            return val;
        }

        static void copy(ReferenceCounter &dest, const ReferenceCounter &src, bool destroy = false) {
            if (&dest != &src) {
                src._mutex->lock();
                if (destroy) {
                    if (src._mutex == dest._mutex) {
                        src._mutex->unlock();
                        dest._destroy();
                        src._mutex->lock();
                    } else {
                        dest._destroy();
                    }
                }
                dest._refs = src._refs;
                dest._mutex = src._mutex;
                *dest._refs = *dest._refs + 1;
                src._mutex->unlock();
            }
        }

    public:
        ReferenceCounter() {
            _refs = new uint64_t;
            _mutex = new std::mutex;
            *_refs = 0;
        }

        ReferenceCounter(const ReferenceCounter &_copy) {
            copy(*this, _copy);
        }

        ~ReferenceCounter() {
            _destroy();
        }
    };

    class Allocator : private ReferenceCounter {
    private:
        memmgr *__rainman_mgr = &rglobalmgr;

        void _destroy() {
            if (!refs() && __rainman_mgr != &rglobalmgr) {
                delete __rainman_mgr;
            }
        }

    public:
        Allocator() = default;

        Allocator(memmgr *mgr) {
            __rainman_mgr = mgr;
        }

        Allocator(const Allocator &copy) : ReferenceCounter(copy) {
            __rainman_mgr = copy.__rainman_mgr;
        }

        Allocator &operator=(const Allocator &rhs) {
            if (this != &rhs) {
                ReferenceCounter::copy(*this, rhs, true);
                __rainman_mgr = rhs.__rainman_mgr;
            }

            return *this;
        }

        template<typename Type>
        Type *rmalloc(uint64_t n) {
            return __rainman_mgr->r_malloc<Type>(n);
        }

        template<typename Type>
        Type *rnew() {
            return __rainman_mgr->r_malloc<Type>(1);
        }

        template<typename Type>
        void rfree(Type *ptr) {
            __rainman_mgr->r_free(ptr);
        }

        ~Allocator() {
            _destroy();
        }
    };
}

#endif
