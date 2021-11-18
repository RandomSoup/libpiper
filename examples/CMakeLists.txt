cmake_minimum_required(VERSION 3.13.0)

project(examples)

# Examples

add_executable(simple-client simple-client/main.c)
target_link_libraries(simple-client piper)

add_executable(simple-server simple-server/main.c)
target_link_libraries(simple-server piper)

add_executable(file-server file-server/main.cpp)
target_link_libraries(file-server piper)
