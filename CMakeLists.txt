cmake_minimum_required(VERSION 3.13.0)

# Clang

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Start

project(libpiper)

# Warnings

add_compile_options(-Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)
add_link_options(-Wl,--no-undefined)
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Main

add_library(piper SHARED src/common.c src/client.c src/server.c)
target_include_directories(piper PUBLIC include)
target_compile_definitions(piper PRIVATE -DPIPER_INTERNAL)

# Examples

add_subdirectory(examples)

# Testing

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build Testing Code" TRUE)
    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
