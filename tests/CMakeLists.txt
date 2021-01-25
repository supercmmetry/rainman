project(rain_man_test)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(gtest_run rain_man.cpp)
target_link_libraries(gtest_run gtest gtest_main rainman)