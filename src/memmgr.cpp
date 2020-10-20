#include "memmgr.h"

rainman::memmgr::memmgr(uint64_t map_size) {
    sem_init(&mutex, 0, 1);
    memmap = new rainman::memmap(map_size);
    n_allocations = 0;
    allocation_size = 0;
    peak_size = 0;
    parent = nullptr;
}

void rainman::memmgr::set_peak(uint64_t _peak_size) {
    lock();
    peak_size = _peak_size;

    if (allocation_size > peak_size) {
        unlock();
        throw MemoryErrors::PeakLimitReachedException();
    }

    unlock();
}

uint64_t rainman::memmgr::get_alloc_count() {
    lock();
    int n = n_allocations;
    unlock();

    return n;
}

uint64_t rainman::memmgr::get_alloc_size() {
    lock();
    int size = allocation_size;
    unlock();

    return size;
}

void rainman::memmgr::set_parent(rainman::memmgr *p) {
    parent = p;
}

uint64_t rainman::memmgr::get_peak_size() {
    lock();
    int size = peak_size;
    unlock();

    return size;
}

void rainman::memmgr::update(uint64_t alloc_size, uint64_t alloc_count) {
    if (parent != nullptr) {
        parent->lock();
        parent->update(parent->allocation_size + alloc_size - allocation_size,
                       parent->n_allocations + alloc_count - n_allocations);
        parent->unlock();
    }

    allocation_size = alloc_size;
    n_allocations = alloc_count;
}

void rainman::memmgr::lock() {
    sem_wait(&mutex);
}

void rainman::memmgr::unlock() {
    sem_post(&mutex);
}

rainman::memmgr *rainman::memmgr::create_child_mgr() {
    auto *mgr = new rainman::memmgr;
    mgr->parent = this;

    lock();
    children[mgr] = true;
    unlock();

    return mgr;
}

void rainman::memmgr::unregister() {
    if (parent != nullptr) {
        parent->lock();
        parent->children.erase(this);
        parent->unlock();
    }
}

rainman::memmgr *rainman::memmgr::get_parent() {
    return parent;
}
