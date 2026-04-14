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

# Collect all .cpp and *.h source files in the current directory, allocator, and tests directory
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h" 
                  "${CMAKE_SOURCE_DIR}/allocator/*.cpp" "${CMAKE_SOURCE_DIR}/allocator/*.h"
                  "${CMAKE_SOURCE_DIR}/tests/*.cpp" "${CMAKE_SOURCE_DIR}/tests/*.h")

# Add include directories
include_directories(allocator)

# Define DMQ_ALLOCATOR
add_definitions(-DDMQ_ALLOCATOR)

# Add an executable target
add_executable(stl_allocator_app ${SOURCES})



