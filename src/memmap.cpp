#include <iostream>
#include "memmap.h"

rain_man_memmap::rain_man_memmap(uint64_t size) {
    mapptr =  new rain_man_map_elem*[size];
    for (int i = 0; i < size; i++) {
        mapptr[i] = nullptr;
    }
    max_size = size;

    iterptr = nullptr;
}

void rain_man_memmap::add(rain_man_map_elem *elem) {
    uint64_t ptr_hash = hash(elem->ptr);
    elem->next = mapptr[ptr_hash];
    mapptr[ptr_hash] = elem;

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

rain_man_map_elem *rain_man_memmap::get(void *ptr) {
    uint64_t ptr_hash = hash(ptr);
    auto curr = mapptr[ptr_hash];

    while (curr != nullptr && curr->ptr != ptr) {
        curr = curr->next;
    }

    return curr;
}

uint64_t rain_man_memmap::hash(void *ptr) {
    auto value = (uint64_t) ptr;
    value = value + value ^ (value >> 2);
    return value % max_size;
}

