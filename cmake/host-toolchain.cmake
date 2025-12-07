# Host Toolchain File for Native Compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/host-toolchain.cmake ..

# This toolchain file uses the system's default compilers for native compilation
# It's useful for testing and development on the host machine

# Use system compiler
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define a flag the source code can check to see if it's being built
# with the host toolchain
add_definitions(-DHOST_TOOLCHAIN)

# Set a CMake variable that can be checked in CMakeLists.txt
set(IS_HOST_BUILD TRUE CACHE BOOL "Building for host platform" FORCE)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_definitions(-DBUILD_TARGET_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	add_definitions(-DBUILD_TARGET_MACOS)
endif()
