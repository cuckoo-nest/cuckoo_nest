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

# Optional: Add host-specific flags
# set(CMAKE_C_FLAGS_INIT "-Wall -Wextra")
# set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra")
