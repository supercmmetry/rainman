#include <iostream>
#include <iomanip>
#include "rainman/memmgr.h"

rainman::memmgr::memmgr(uint64_t map_size) {
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
    auto n = n_allocations;
    unlock();

    return n;
}

uint64_t rainman::memmgr::get_alloc_size() {
    lock();
    auto size = allocation_size;
    unlock();

    return size;
}

void rainman::memmgr::set_parent(rainman::memmgr *p) {
    parent = p;
}

uint64_t rainman::memmgr::get_peak_size() {
    lock();
    auto size = peak_size;
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
    mutex.lock();
}

void rainman::memmgr::unlock() {
    mutex.unlock();
}

rainman::memmgr *rainman::memmgr::create_child_mgr() {
    auto *mgr = new rainman::memmgr;

    lock();
    mgr->parent = this;
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

void rainman::memmgr::print_mem_trace() {
    lock();

    std::cout << "Rainman Memory Trace:" << std::endl << std::endl;
    std::cout << std::setw(40) << std::left << "Type name (RTTI)"
              << std::setw(20) << std::left << "Allocation size" << std::endl;

    auto iter = memmap->iterptr;

    while (iter != nullptr) {
        std::cout << std::setw(40) << std::left << iter->type_name
                  << std::setw(20) << std::left << std::to_string(iter->alloc_size) + " bytes" << std::endl;

        iter = iter->prev_iter;
    }

    std::cout << std::endl;
    std::cout << "Overall stats: " << std::endl << std::endl;
    std::cout << "Allocation size: " << allocation_size << " bytes" << std::endl;
    std::cout << "Allocation count: " << n_allocations << std::endl;

    unlock();
}
