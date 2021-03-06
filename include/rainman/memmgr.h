#ifndef RAINMAN_MEMMGR_H
#define RAINMAN_MEMMGR_H

#include <cstdint>
#include <cstring>
#include <semaphore.h>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include "errors.h"
#include "memmap.h"

namespace rainman {
    class memmgr {
    private:
        uint64_t _allocation_size{};
        uint64_t _n_allocations{};
        uint64_t _peak_size{};
        memmap *_memmap{};
        memmgr *_parent{};
        std::unordered_map<memmgr *, bool> _children{};
        std::mutex _mutex{};

        void lock();

        void unlock();

        void update(uint64_t alloc_size, uint64_t alloc_count);

    public:
        memmgr(uint64_t map_size = 0xffff);

        ~memmgr() {
            lock();
            delete _memmap;
            unlock();
        }

        template<typename Type>
        Type *r_malloc(uint64_t n_elems) {
            lock();

            uint64_t curr_alloc_size = sizeof(Type) * n_elems;

            if (_peak_size != 0 && _allocation_size + curr_alloc_size > _peak_size) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            if (_parent != nullptr &&
                _parent->_peak_size != 0 &&
                _parent->get_alloc_size() + curr_alloc_size > _parent->get_peak_size()) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            auto elem = new map_elem;

            elem->ptr = new Type[n_elems];
            elem->alloc_size = n_elems * sizeof(Type);
            elem->count = n_elems;
            elem->type_name = typeid(Type).name();
            elem->next = nullptr;

            _memmap->add(elem);

            update(_allocation_size + elem->alloc_size, _n_allocations + 1);

            unlock();

            return static_cast<Type*>(elem->ptr);
        }

        template<typename Type>
        void r_free(Type *ptr) {
            if (ptr == nullptr) {
                return;
            }

            lock();

            auto *elem = _memmap->get((void *) ptr);
            if (elem != nullptr) {
                update(_allocation_size - elem->alloc_size, _n_allocations - 1);
                unlock();
                _memmap->remove_by_type<Type>(ptr);
                lock();
            } else {
                unlock();
                for (auto child : _children) {
                    child.first->r_free(ptr);
                }
                lock();
            }

            unlock();
        }

        template<typename Type, typename ...Args>
        Type *r_new(uint64_t n_elems, Args ...args) {
            lock();

            uint64_t curr_alloc_size = sizeof(Type) * n_elems;

            if (_peak_size != 0 && _allocation_size + curr_alloc_size > _peak_size) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            if (_parent != nullptr &&
                _parent->_peak_size != 0 &&
                _parent->get_alloc_size() + curr_alloc_size > _parent->get_peak_size()) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            auto elem = new map_elem;

            elem->alloc_size = n_elems * sizeof(Type);
            elem->count = n_elems;
            elem->ptr = operator new[](elem->alloc_size);
            elem->type_name = typeid(Type).name();
            elem->next = nullptr;
            elem->is_raw = true;

            _memmap->add(elem);

            update(_allocation_size + elem->alloc_size, _n_allocations + 1);

            unlock();

            Type *objects = reinterpret_cast<Type*>(elem->ptr);

            for (uint64_t i = 0; i < n_elems; i++) {
                new(objects + i) Type(std::forward<Args>(args)...);
            }

            return objects;
        }

        void set_peak(uint64_t _peak_size);

        void set_parent(memmgr *p);

        memmgr *get_parent();

        void unregister();

        uint64_t get_alloc_count();

        uint64_t get_alloc_size();

        void print_mem_trace();

        uint64_t get_peak_size();

        memmgr *create_child_mgr();

        // De-allocate everything allocated by the memory manager by type.
        template<typename Type>
        void wipe(bool deep_wipe = false) {
            lock();

            auto *curr = _memmap->head;
            while (curr != nullptr) {
                auto next = curr->next_iter;
                auto ptr = curr->ptr;
                if (ptr == nullptr) {
                    curr = next;
                    continue;
                }

                auto *elem = _memmap->get((void *) ptr);
                if (elem != nullptr) {
                    if (strcmp(typeid(Type).name(), elem->type_name) == 0) {
                        update(_allocation_size - elem->alloc_size, _n_allocations - 1);
                        unlock();
                        _memmap->remove_by_type<Type *>(reinterpret_cast<Type *>(ptr));
                        lock();
                    }
                }

                curr = next;
            }

            unlock();

            if (deep_wipe) {
                for (auto &child : _children) {
                    child.first->wipe<Type>();
                }
            }
        }
    };
}


#endif
