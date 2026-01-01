# ARM Linaro Toolchain File for Cross-Compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Linaro Toolchain settings
set(LINARO_VERSION "gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi")
set(LINARO_URL "https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/arm-linux-gnueabi/${LINARO_VERSION}.tar.xz")
set(LINARO_DIR "${CMAKE_BINARY_DIR}/linaro_toolchain")
set(LINARO_BIN "${LINARO_DIR}/bin")
set(LINARO_TARBALL "${CMAKE_BINARY_DIR}/${LINARO_VERSION}.tar.xz")

# Device sysroot repository
set(DEVICE_SYSROOT_REPO "https://github.com/cuckoo-nest/sysroot.git")
set(DEVICE_SYSROOT "${CMAKE_BINARY_DIR}/device_sysroot")

# Download and extract toolchain if not already present
if(NOT EXISTS "${LINARO_BIN}/arm-linux-gnueabi-gcc")
    message(STATUS "Linaro toolchain not found, downloading...")
    
    if(NOT EXISTS "${LINARO_TARBALL}")
        file(DOWNLOAD "${LINARO_URL}" "${LINARO_TARBALL}"
             SHOW_PROGRESS
             STATUS DOWNLOAD_STATUS)
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        if(NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "Failed to download Linaro toolchain from ${LINARO_URL}")
        endif()
    endif()
    
    message(STATUS "Extracting Linaro toolchain...")
    file(ARCHIVE_EXTRACT INPUT "${LINARO_TARBALL}" DESTINATION "${CMAKE_BINARY_DIR}")
    file(RENAME "${CMAKE_BINARY_DIR}/${LINARO_VERSION}" "${LINARO_DIR}")
    message(STATUS "Linaro toolchain extracted to ${LINARO_DIR}")
endif()

# Clone device sysroot if not present
if(NOT EXISTS "${DEVICE_SYSROOT}/.git")
    message(STATUS "Device sysroot not found, cloning from ${DEVICE_SYSROOT_REPO}...")
    execute_process(
        COMMAND git clone --depth 1 ${DEVICE_SYSROOT_REPO} ${DEVICE_SYSROOT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE SYSROOT_CLONE_RESULT
    )
    if(NOT SYSROOT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone device sysroot from ${DEVICE_SYSROOT_REPO}")
    endif()
endif()

# Set compilers
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-g++)

# Set sysroot for cross-compilation
# Hybrid approach: Use Linaro for build-time files (crt*.o, headers)
# but link against device libraries for runtime compatibility
set(LINARO_SYSROOT "${LINARO_DIR}/arm-linux-gnueabi/libc")

# Use Linaro sysroot for build (provides crt*.o startup files)
set(CMAKE_SYSROOT ${LINARO_SYSROOT})
set(CMAKE_FIND_ROOT_PATH ${LINARO_SYSROOT})

# Add device sysroot to library search paths (takes precedence for runtime libs)
set(CMAKE_LIBRARY_PATH 
    "${DEVICE_SYSROOT}/lib"
    "${DEVICE_SYSROOT}/usr/lib"
    "${CMAKE_SYSROOT}/lib"
    "${CMAKE_SYSROOT}/usr/lib"
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
set(CMAKE_C_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfloat-abi=soft")
set(CMAKE_CXX_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfloat-abi=soft")

# Add linker flags: prioritize device libraries, use Linaro for startup files
# -L flags are searched in order, so device libs are found first
set(CMAKE_EXE_LINKER_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -L${DEVICE_SYSROOT}/lib -L${DEVICE_SYSROOT}/usr/lib -Wl,-rpath-link,${DEVICE_SYSROOT}/lib:${DEVICE_SYSROOT}/usr/lib")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "--sysroot=${CMAKE_SYSROOT} -L${DEVICE_SYSROOT}/lib -L${DEVICE_SYSROOT}/usr/lib -Wl,-rpath-link,${DEVICE_SYSROOT}/lib:${DEVICE_SYSROOT}/usr/lib")

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBUILD_TARGET_LINUX)
add_definitions(-DBUILD_TARGET_NEST)
