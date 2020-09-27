cmake_minimum_required(VERSION 3.17)
project(rain_man)

set(CMAKE_CXX_STANDARD 20)

include_directories("src")

add_library(rain_man STATIC
        src/errors.h
        src/mem_interface.h
        src/memmgr.h
        src/memmgr.cpp
        src/memmap.h
        src/memmap.cpp)

enable_testing()
add_subdirectory(tests)