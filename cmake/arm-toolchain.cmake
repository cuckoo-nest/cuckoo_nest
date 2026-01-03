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
#set(DEVICE_SYSROOT "${CMAKE_BINARY_DIR}/device_sysroot")
set(DEVICE_SYSROOT "/home/alex/nest-sysroot")

# # Verify toolchain exists
# if(NOT EXISTS "${LINARO_BIN}/arm-none-linux-gnueabi-gcc")
#     message(FATAL_ERROR "ARM toolchain not found at ${LINARO_BIN}/arm-none-linux-gnueabi-gcc")
# endif()

# # Clone device sysroot if not present
# if(NOT EXISTS "${DEVICE_SYSROOT}/.git")
#     message(STATUS "Device sysroot not found, cloning from ${DEVICE_SYSROOT_REPO}...")
#     execute_process(
#         COMMAND git clone --depth 1 ${DEVICE_SYSROOT_REPO} ${DEVICE_SYSROOT}
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         RESULT_VARIABLE SYSROOT_CLONE_RESULT
#     )
#     if(NOT SYSROOT_CLONE_RESULT EQUAL 0)
#         message(FATAL_ERROR "Failed to clone device sysroot from ${DEVICE_SYSROOT_REPO}")
#     endif()
# endif()

# Set compilers
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabihf-g++)

# Set sysroot for cross-compilation
# Hybrid approach: Use Linaro for build-time files (crt*.o, headers)
# but link against device libraries for runtime compatibility
#set(LINARO_SYSROOT "${LINARO_DIR}/arm-linux-gnueabi/libc")

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

# Add linker flags: explicitly link libstdc++ from device sysroot first
# Link device libraries first, then fall back to Linaro for any missing symbols
#set(CMAKE_EXE_LINKER_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -L${DEVICE_SYSROOT}/usr/lib -L${DEVICE_SYSROOT}/lib -L${LINARO_SYSROOT}/usr/lib -Wl,-rpath-link,${DEVICE_SYSROOT}/lib:${DEVICE_SYSROOT}/usr/lib:${LINARO_SYSROOT}/lib")
#set(CMAKE_SHARED_LINKER_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -L${DEVICE_SYSROOT}/usr/lib -L${DEVICE_SYSROOT}/lib -L${LINARO_SYSROOT}/usr/lib -Wl,-rpath-link,${DEVICE_SYSROOT}/lib:${DEVICE_SYSROOT}/usr/lib:${LINARO_SYSROOT}/lib")

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBUILD_TARGET_LINUX)
add_definitions(-DBUILD_TARGET_NEST)
