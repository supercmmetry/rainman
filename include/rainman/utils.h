#ifndef RAINMAN_UTILS_H
#define RAINMAN_UTILS_H

#include <cstdint>
#include <atomic>
#include <memory>
#include <rainman/memmgr.h>

namespace rainman {
    class ReferenceCounter {
    private:
        std::shared_ptr<std::atomic<uint64_t>> _refs;

        void _destroy() {
            if (*_refs > 0) {
                (*_refs)--;
            }
        }

    protected:
        uint64_t refs() {
            return *_refs;
        }

        static void copy(ReferenceCounter &dest, const ReferenceCounter &src, bool destroy = false) {
            if (&dest != &src) {
                if (destroy) {
                    dest._destroy();
                }
                dest._refs = src._refs;
                (*dest._refs)++;
            }
        }

    public:
        ReferenceCounter() {
            _refs = std::make_shared<std::atomic<uint64_t>>();
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
        static rainman::memmgr _default_mgr;
        memmgr *_rainman_mgr = &_default_mgr;

        void _destroy() {
            if (!refs() && _rainman_mgr != &_default_mgr) {
                delete _rainman_mgr;
            }
        }

    public:
        Allocator() = default;

        Allocator(memmgr *mgr) {
            _rainman_mgr = mgr;
        }

        Allocator(const Allocator &copy) : ReferenceCounter(copy) {
            _rainman_mgr = copy._rainman_mgr;
        }

        Allocator &operator=(const Allocator &rhs) {
            if (this != &rhs) {
                ReferenceCounter::copy(*this, rhs, true);
                _rainman_mgr = rhs._rainman_mgr;
            }

            return *this;
        }

        template<typename Type>
        Type *rmalloc(uint64_t n) {
            return _rainman_mgr->r_malloc<Type>(n);
        }

        template<typename Type, typename ...Args>
        Type *rnew(uint64_t n_elems, Args ...args) {
            return _rainman_mgr->r_new<Type>(n_elems, std::forward<Args>(args)...);
        }

        template<typename Type>
        void rfree(Type *ptr) {
            _rainman_mgr->r_free(ptr);
        }

        ~Allocator() {
            _destroy();
        }
    };
}

#endif
