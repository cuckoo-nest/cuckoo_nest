cmake_minimum_required(VERSION 3.16)

# Use old linker behavior for compatibility with older toolchains
if(POLICY CMP0116)
    cmake_policy(SET CMP0116 OLD)
endif()


project(CuckooNest CXX)

include(ExternalProject)

# =====================================================
# 1. Define the toolchain download information
# =====================================================
set(LINARO_URL "https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/arm-linux-gnueabi/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi.tar.xz")
set(LINARO_TARBALL "${CMAKE_BINARY_DIR}/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi.tar.xz")
set(LINARO_DIR "${CMAKE_BINARY_DIR}/linaro/src/linaro_toolchain")
set(LINARO_BIN "${LINARO_DIR}/bin")

# =====================================================
# 2. Download & extract the toolchain using ExternalProject
# =====================================================
ExternalProject_Add(
    linaro_toolchain
    PREFIX ${CMAKE_BINARY_DIR}/linaro
    URL ${LINARO_URL}
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    CONFIGURE_COMMAND ""   # no configure step
    BUILD_COMMAND ""       # no build step
    INSTALL_COMMAND ""     # no install step
    UPDATE_DISCONNECTED 1
)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("lvgl")
add_subdirectory("src")
add_subdirectory("tests")