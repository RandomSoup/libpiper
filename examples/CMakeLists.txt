cmake_minimum_required(VERSION 3.13.0)

project(examples)

# Examples

add_executable(simple-client.o simple-client/main.c)
target_link_libraries(simple-client.o piper)

add_executable(simple-server.o simple-server/main.c)
target_link_libraries(simple-server.o piper)

add_executable(file-server.o file-server/main.cpp)
target_link_libraries(file-server.o piper)
