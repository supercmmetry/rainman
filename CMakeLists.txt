cmake_minimum_required(VERSION 3.17)
project(rainman)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("src")

add_library(rainman STATIC
        src/errors.h
        src/rainman.h
        src/memmgr.h
        src/memmgr.cpp
        src/memmap.h
        src/memmap.cpp src/context.h)

add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)