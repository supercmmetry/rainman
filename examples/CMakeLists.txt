project(rain_man_examples)

add_executable(rainman_example rain_man.cpp)
target_link_libraries(rainman_example pthread rainman)