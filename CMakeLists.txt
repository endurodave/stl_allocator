# Example CMake command line to create project build files:
#
# *** Windows ***
# cmake -G "Visual Studio 17 2022" -A Win32 -B build -S .
#
# *** Linux ***
# cmake -G "Unix Makefiles" -B build -S .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language (C or C++)
project(stl_allocator VERSION 1.0 LANGUAGES CXX)

# The library headers (src/, the Arduino library layout), the copies of
# Allocator and xallocator they build on (allocator/), and the desktop demo
# and unit tests (examples/desktop)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.h"
                  "${CMAKE_SOURCE_DIR}/allocator/*.cpp" "${CMAKE_SOURCE_DIR}/allocator/*.h"
                  "${CMAKE_SOURCE_DIR}/examples/desktop/*.cpp" "${CMAKE_SOURCE_DIR}/examples/desktop/*.h")

# Add include directories
include_directories(src allocator)

# Define DMQ_ALLOCATOR
add_definitions(-DDMQ_ALLOCATOR)

# Add an executable target
add_executable(stl_allocator_app ${SOURCES})



