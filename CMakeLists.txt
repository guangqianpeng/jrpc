cmake_minimum_required(VERSION 2.6)
project(jrpc)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
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
        third_party/tinyev
        third_party/jackson
        ${PROJECT_SOURCE_DIR})

add_subdirectory(third_party/tinyev)
add_subdirectory(third_party/jackson)
add_subdirectory(jrpc)

add_executable(server server.cc HelloServerStub.h)
target_link_libraries(server jrpc)

add_executable(client client.cc HelloClientStub.h)
target_link_libraries(client jrpc)