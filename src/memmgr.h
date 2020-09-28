#ifndef RAINMAN_MEMMGR_H
#define RAINMAN_MEMMGR_H

#include <cstdint>
#include <cstring>
#include <semaphore.h>
#include <vector>
#include <typeinfo>
#include "errors.h"
#include "memmap.h"

namespace rainman {
    class memmgr {
    private:
        uint64_t allocation_size;
        uint64_t n_allocations;
        uint64_t peak_size;
        memmap *memmap;
        memmgr *parent;
        std::vector<memmgr *> children;
        sem_t mutex{};

        void lock();

        void unlock();

        void update(uint64_t alloc_size, uint64_t alloc_count);

    public:
        memmgr(uint64_t map_size = 0xffff);

        ~memmgr() {
            sem_wait(&mutex);

            delete memmap;

            sem_post(&mutex);
        }

        template<typename Type>
        Type *r_malloc(int n_elems) {
            lock();

            uint64_t curr_alloc_size = sizeof(Type) * n_elems;

            if (peak_size != 0 && allocation_size + curr_alloc_size > peak_size) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            if (parent != nullptr &&
                parent->peak_size != 0 &&
                parent->get_alloc_size() + curr_alloc_size > parent->get_peak_size()) {
                unlock();
                throw MemoryErrors::PeakLimitReachedException();
            }

            auto elem = new map_elem;

            elem->ptr = new Type[n_elems];
            elem->alloc_size = n_elems * sizeof(Type);
            elem->type_name = typeid(Type).name();
            elem->next = nullptr;

            memmap->add(elem);

            update(allocation_size + elem->alloc_size, n_allocations + 1);

            unlock();

            return (Type *) elem->ptr;
        }

        template<typename Type>
        void r_free(Type ptr) {
            if (ptr == nullptr) {
                return;
            }

            lock();

            auto *elem = memmap->get((void *) ptr);
            if (elem != nullptr) {
                update(allocation_size - elem->alloc_size, n_allocations - 1);
                memmap->remove_by_type<Type>(ptr);
            }

            unlock();
        }

        void set_peak(uint64_t _peak_size);

        void set_parent(memmgr *p);

        uint64_t get_alloc_count();

        uint64_t get_alloc_size();

        uint64_t get_peak_size();

        memmgr *create_child_mgr();

        // De-allocate everything allocated by the memory manager.
        // All child memory-managers are wiped in the process.
        // Note that this does not call the destructor of the allocated objects.
        template<typename Type>
        void wipe(bool wipe_children = true) {
            lock();

            auto *curr = memmap->head;
            while (curr != nullptr) {
                auto next = curr->next_iter;
                auto ptr = curr->ptr;
                if (ptr == nullptr) {
                    curr = next;
                    continue;
                }

                auto *elem = memmap->get((void *) ptr);
                if (elem != nullptr) {
                    if (strcmp(typeid(Type).name(), typeid(void).name()) == 0) {
                        update(allocation_size - elem->alloc_size, n_allocations - 1);
                        memmap->remove_by_type<void*>(ptr);
                    } else if (strcmp(typeid(Type).name(), elem->type_name) == 0) {
                        update(allocation_size - elem->alloc_size, n_allocations - 1);
                        memmap->remove_by_type<Type*>((Type*)ptr);
                    }
                }

                curr = next;
            }

            unlock();

            if (wipe_children) {
                for (auto &child : children) {
                    child->wipe<Type>();
                    delete child;
                }

                children.clear();
            }
        }
    };
}


#endif
