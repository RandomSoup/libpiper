cmake_minimum_required(VERSION 3.13.0)

project(tests)

# Testing

add_executable(url-parsing src/url-parsing.c)
target_link_libraries(url-parsing piper)
add_test(NAME url-parsing COMMAND url-parsing)

add_executable(url-resolution src/url-resolution.c)
target_link_libraries(url-resolution piper)
add_test(NAME url-resolution COMMAND url-resolution)

add_executable(basic src/basic.c)
target_link_libraries(basic piper pthread)
add_test(NAME basic COMMAND basic)
