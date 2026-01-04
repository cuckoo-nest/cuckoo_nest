# ARM Linaro Toolchain File for Cross-Compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ARM Toolchain settings
# Using Linaro arm-linux-gnueabihf 4.8-2014.04 toolchain
set(LINARO_DIR "${CMAKE_SOURCE_DIR}/linaro_toolchain/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux")
set(LINARO_BIN "${LINARO_DIR}/bin")
set(LINARO_SYSROOT "${LINARO_DIR}/arm-linux-gnueabihf/libc")

# Download and extract the toolchain if not present
if(NOT EXISTS "${LINARO_DIR}")
    message(STATUS "Linaro toolchain not found. Downloading...")
    
    set(TOOLCHAIN_URL "https://releases.linaro.org/archive/14.04/components/toolchain/binaries/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz")
    set(TOOLCHAIN_ARCHIVE "${CMAKE_SOURCE_DIR}/linaro_toolchain/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz")
    set(TOOLCHAIN_DIR "${CMAKE_SOURCE_DIR}/linaro_toolchain")
    
    # Create toolchain directory if it doesn't exist
    file(MAKE_DIRECTORY "${TOOLCHAIN_DIR}")
    
    # Download the toolchain
    if(NOT EXISTS "${TOOLCHAIN_ARCHIVE}")
        message(STATUS "Downloading toolchain from ${TOOLCHAIN_URL}")
        file(DOWNLOAD "${TOOLCHAIN_URL}" "${TOOLCHAIN_ARCHIVE}"
            SHOW_PROGRESS
            TIMEOUT 300
        )
    endif()
    
    # Extract the toolchain
    message(STATUS "Extracting toolchain to ${TOOLCHAIN_DIR}")
    execute_process(
        COMMAND tar -xf "${TOOLCHAIN_ARCHIVE}" -C "${TOOLCHAIN_DIR}"
        RESULT_VARIABLE extract_result
    )
    
    if(NOT extract_result EQUAL 0)
        message(FATAL_ERROR "Failed to extract toolchain archive")
    endif()
    
    if(NOT EXISTS "${LINARO_DIR}")
        message(FATAL_ERROR "Toolchain extraction failed: ${LINARO_DIR} not found")
    endif()
    
    message(STATUS "Toolchain successfully downloaded and extracted to ${LINARO_DIR}")
endif()


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
