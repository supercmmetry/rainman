project(rainman_test)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(rainman_test rain_man.cpp)
target_link_libraries(rainman_test gtest gtest_main rainman)