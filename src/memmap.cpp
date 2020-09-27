#include <iostream>
#include "memmap.h"

rainman::memmap::memmap(uint64_t size) {
    mapptr =  new map_elem*[size];
    for (int i = 0; i < size; i++) {
        mapptr[i] = nullptr;
    }
    max_size = size;

    iterptr = nullptr;
}

void rainman::memmap::add(map_elem *elem) {
    uint64_t ptr_hash = hash(elem->ptr);
    elem->next = mapptr[ptr_hash];
    mapptr[ptr_hash] = elem;
    elem->next = nullptr;

    if (iterptr == nullptr) {
        iterptr = elem;
        head = elem;
        iterptr->next_iter = nullptr;
        iterptr->prev_iter = nullptr;
    } else {
        iterptr->next_iter = elem;
        elem->prev_iter = iterptr;
        iterptr = iterptr->next_iter;
        iterptr->next_iter = nullptr;
    }
}

rainman::map_elem *rainman::memmap::get(void *ptr) {
    uint64_t ptr_hash = hash(ptr);
    auto curr = mapptr[ptr_hash];

    while (curr != nullptr && curr->ptr != ptr) {
        curr = curr->next;
    }

    if (curr->ptr != ptr) {
        return nullptr;
    }

    return curr;
}

uint64_t rainman::memmap::hash(void *ptr) {
    auto value = (uint64_t) ptr;
    value = value + value ^ (value >> 2);
    return value % max_size;
}

