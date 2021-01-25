cmake_minimum_required(VERSION 3.17)
project(rainman LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(rainman SHARED
        src/memmgr.cpp
        src/memmap.cpp
        src/cache.cpp)

target_include_directories(rainman
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)