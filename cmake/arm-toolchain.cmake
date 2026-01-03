# ARM Linaro Toolchain File for Cross-Compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ARM Toolchain settings
# Using Linaro arm-linux-gnueabihf 4.8-2014.04 toolchain
set(LINARO_DIR "/home/alex/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux")
set(LINARO_BIN "${LINARO_DIR}/bin")
set(LINARO_SYSROOT "${LINARO_DIR}/arm-linux-gnueabihf/libc")

# Device sysroot repository
set(DEVICE_SYSROOT_REPO "https://github.com/cuckoo-nest/sysroot.git")
set(DEVICE_SYSROOT "/home/alex/nest-sysroot")


# Set compilers
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabihf-g++)

# Use Linaro sysroot for build (provides crt*.o startup files)
set(CMAKE_SYSROOT ${LINARO_SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${LINARO_SYSROOT};)

# Add device sysroot to library search paths (takes precedence for runtime libs)
set(CMAKE_LIBRARY_PATH 
    "${LINARO_SYSROOT}/lib"
    "${LINARO_SYSROOT}/usr/lib"
    "${DEVICE_SYSROOT}/lib"
    "${DEVICE_SYSROOT}/usr/lib"
)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Disable dependency file generation for older toolchains
set(CMAKE_CXX_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_C_DEPENDS_USE_COMPILER FALSE)

# Set common ARM flags
set(CMAKE_C_FLAGS_INIT "-march=armv7-a -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS_INIT "-march=armv7-a -mfloat-abi=hard")

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBUILD_TARGET_LINUX)
add_definitions(-DBUILD_TARGET_NEST)
