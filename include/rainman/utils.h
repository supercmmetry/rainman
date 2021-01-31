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

        explicit Allocator(memmgr *mgr) {
            _rainman_mgr = mgr;
        }

    public:
        Allocator() = default;

        explicit Allocator(uint64_t map_size) {
            _rainman_mgr = new rainman::memmgr(map_size);
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
        inline Type *rmalloc(uint64_t n) {
            return _rainman_mgr->r_malloc<Type>(n);
        }

        template<typename Type, typename ...Args>
        inline Type *rnew(uint64_t n_elems, Args ...args) {
            return _rainman_mgr->r_new<Type>(n_elems, std::forward<Args>(args)...);
        }

        template<typename Type>
        inline void rfree(Type *ptr) {
            _rainman_mgr->r_free(ptr);
        }

        inline uint64_t alloc_size() {
            return _rainman_mgr->get_alloc_size();
        }

        inline uint64_t alloc_count() {
            return _rainman_mgr->get_alloc_count();
        }

        inline uint64_t peak_size() {
            return _rainman_mgr->get_peak_size();
        }

        inline void peak_size(uint64_t _peak_size) {
            _rainman_mgr->set_peak(_peak_size);
        }

        inline void mem_trace() {
            _rainman_mgr->print_mem_trace();
        }

        inline Allocator create_child() {
            return Allocator(_rainman_mgr->create_child_mgr());
        }

        inline void unregister() {
            _rainman_mgr->unregister();
        }

        template <typename Type>
        inline void wipe(bool deep_wipe = false) {
            _rainman_mgr->wipe<Type>(deep_wipe);
        }

        ~Allocator() {
            _destroy();
        }
    };
}

#endif
