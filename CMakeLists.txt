cmake_minimum_required(VERSION 2.6)
project(jrpc)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT CMAKE_STUB_FORMATTER)
    set(CMAKE_STUB_FORMATTER echo)
endif()

set(CXX_FLAGS
        -fno-omit-frame-pointer # linux perf
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -std=c++17
        -march=native
        -rdynamic)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(
        3rd/tinyev
        3rd/jackson
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR})

add_subdirectory(3rd/tinyev)
add_subdirectory(3rd/jackson)
add_subdirectory(jrpc)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(example)
endif()
