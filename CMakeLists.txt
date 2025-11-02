cmake_minimum_required(VERSION 3.20)
project(ARM_CrossBuild CXX)

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

# =====================================================
# 3. Define your actual build target
# =====================================================
set (
    SRC_FILES 
    src/main.cpp
    src/HAL/Beeper.cpp
)

add_executable(hello ${SRC_FILES})

# Tell CMake that this target depends on the toolchain being ready
add_dependencies(hello linaro_toolchain)

# =====================================================
# 4. Set the cross-compiler for this project
# =====================================================
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-g++)
set(CMAKE_FIND_ROOT_PATH ${LINARO_DIR}/arm-linux-gnueabi/libc)
set(CMAKE_SYSROOT ${CMAKE_FIND_ROOT_PATH})

# Optional: Add flags, sysroot, etc.
target_compile_options(hello PRIVATE
    --sysroot=${CMAKE_SYSROOT}
    -march=armv7-a
)

# =====================================================
# 5. Output location
# =====================================================
set_target_properties(hello PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output
)

