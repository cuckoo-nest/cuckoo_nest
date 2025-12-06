cmake_minimum_required(VERSION 3.16)
enable_testing()

# Use old linker behavior for compatibility with older toolchains
if(POLICY CMP0116)
    cmake_policy(SET CMP0116 OLD)
endif()

project(CuckooNest CXX)

include(ExternalProject)
include(FetchContent)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("lvgl")
add_subdirectory("src")
add_subdirectory("tests")