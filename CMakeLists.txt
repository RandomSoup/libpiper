cmake_minimum_required(VERSION 3.13.0)

# Clang

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Start

project(libpiper)

# Version

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")

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
set_target_properties(piper PROPERTIES SOVERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Install

include(GNUInstallDirs)

install(TARGETS piper DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT "main")
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT "dev")

# CPack

set(CPACK_PACKAGE_NAME "libpiper")
set(CPACK_PACKAGE_VENDOR "TheBrokenRail")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION "Simple C Library Implementing Piper")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/RandomSoup/libpiper.git")
set(CPACK_PACKAGE_CONTACT "connor24nolan@live.com")
set(CPACK_COMPONENT_DEV_DEPENDS "main")
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "libpiper")
set(CPACK_DEBIAN_MAIN_PACKAGE_NAME "libpiper")
set(CPACK_DEBIAN_DEV_PACKAGE_NAME "libpiper-dev")
set(CPACK_DEB_COMPONENT_INSTALL TRUE)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS TRUE)
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS TRUE)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS TRUE)

include(CPack)

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
