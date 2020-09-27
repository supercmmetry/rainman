cmake_minimum_required(VERSION 3.17)
project(rain_man)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("src")

add_library(rain_man STATIC
        src/errors.h
        src/rainman.h
        src/memmgr.h
        src/memmgr.cpp
        src/memmap.h
        src/memmap.cpp)

enable_testing()
add_subdirectory(tests)