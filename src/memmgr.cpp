#include "memmgr.h"

rain_man_mgr::rain_man_mgr(uint64_t map_size) {
    sem_init(&mutex, 0, 1);
    memmap = new rain_man_memmap(map_size);
    n_allocations = 0;
    allocation_size = 0;
    peak_size = 0;
    parent = nullptr;
}

void rain_man_mgr::set_peak(uint64_t _peak_size) {
    lock();
    peak_size = _peak_size;

    if (allocation_size > peak_size) {
        unlock();
        throw MemoryErrors::PeakLimitReachedException();
    }

    unlock();
}

uint64_t rain_man_mgr::get_alloc_count() {
    lock();
    int n = n_allocations;
    unlock();

    return n;
}

uint64_t rain_man_mgr::get_alloc_size() {
    lock();
    int size = allocation_size;
    unlock();

    return size;
}

void rain_man_mgr::set_parent(rain_man_mgr *p) {
    parent = p;
}

uint64_t rain_man_mgr::get_peak_size() {
    lock();
    int size = peak_size;
    unlock();

    return size;
}

void rain_man_mgr::update(uint64_t alloc_size, uint64_t alloc_count) {
    lock();

    allocation_size = alloc_size;
    n_allocations = alloc_count;

    unlock();
}

void rain_man_mgr::lock() {
    sem_wait(&mutex);
}

void rain_man_mgr::unlock() {
    sem_post(&mutex);
}

void rain_man_mgr::wipe() {
    memmap->clear();
    for (auto & child : children) {
        child->wipe();
        child->~rain_man_mgr();
    }

    children.clear();
}

rain_man_mgr *rain_man_mgr::create_child_mgr() {
    auto *mgr = new rain_man_mgr;
    mgr->parent = this;

    lock();
    children.push_back(mgr);
    unlock();

    return mgr;
}
