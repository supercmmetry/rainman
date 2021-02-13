#include <iostream>
#include <iomanip>
#include "rainman/memmgr.h"

rainman::memmgr::memmgr(uint64_t map_size) {
    _memmap = new rainman::memmap(map_size);
    _n_allocations = 0;
    _allocation_size = 0;
    _peak_size = 0;
    _parent = nullptr;
}

void rainman::memmgr::set_peak(uint64_t _peak_size) {
    lock();
    _peak_size = _peak_size;

    if (_allocation_size > _peak_size) {
        unlock();
        throw MemoryErrors::PeakLimitReachedException();
    }

    unlock();
}

uint64_t rainman::memmgr::get_alloc_count() {
    lock();
    auto n = _n_allocations;
    unlock();

    return n;
}

uint64_t rainman::memmgr::get_alloc_size() {
    lock();
    auto size = _allocation_size;
    unlock();

    return size;
}

void rainman::memmgr::set_parent(rainman::memmgr *p) {
    _parent = p;
}

uint64_t rainman::memmgr::get_peak_size() {
    lock();
    auto size = _peak_size;
    unlock();

    return size;
}

void rainman::memmgr::update(uint64_t alloc_size, uint64_t alloc_count) {
    if (_parent != nullptr) {
        _parent->lock();
        _parent->update(_parent->_allocation_size + alloc_size - _allocation_size,
                        _parent->_n_allocations + alloc_count - _n_allocations);
        _parent->unlock();
    }

    _allocation_size = alloc_size;
    _n_allocations = alloc_count;
}

void rainman::memmgr::lock() {
    _mutex.lock();
}

void rainman::memmgr::unlock() {
    _mutex.unlock();
}

rainman::memmgr *rainman::memmgr::create_child_mgr() {
    auto *mgr = new rainman::memmgr;

    lock();
    mgr->_parent = this;
    _children[mgr] = true;
    unlock();

    return mgr;
}

void rainman::memmgr::unregister() {
    if (_parent != nullptr) {
        _parent->lock();
        _parent->_children.erase(this);
        _parent->unlock();
    }
}

rainman::memmgr *rainman::memmgr::get_parent() {
    return _parent;
}

void rainman::memmgr::print_mem_trace() {
    lock();

    std::cout << "Rainman Memory Trace:" << std::endl << std::endl;
    std::cout << std::setw(40) << std::left << "Type name (RTTI)"
              << std::setw(20) << std::left << "Allocation size" << std::endl;

    auto iter = _memmap->iterptr;

    while (iter != nullptr) {
        std::cout << std::setw(40) << std::left << iter->type_name
                  << std::setw(20) << std::left << std::to_string(iter->alloc_size) + " bytes" << std::endl;

        iter = iter->prev_iter;
    }

    std::cout << std::endl;
    std::cout << "Overall stats: " << std::endl << std::endl;
    std::cout << "Allocation size: " << _allocation_size << " bytes" << std::endl;
    std::cout << "Allocation count: " << _n_allocations << std::endl << std::endl;

    unlock();
}
