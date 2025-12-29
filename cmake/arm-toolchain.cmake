# ARM Linaro Toolchain File for Cross-Compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# =====================================================
# Download Linaro Toolchain if not present
# =====================================================
set(LINARO_VERSION "gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabi")
set(LINARO_URL "https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/arm-linux-gnueabi/${LINARO_VERSION}.tar.xz")
set(LINARO_DIR "${CMAKE_BINARY_DIR}/linaro_toolchain")
set(LINARO_BIN "${LINARO_DIR}/bin")
set(LINARO_TARBALL "${CMAKE_BINARY_DIR}/${LINARO_VERSION}.tar.xz")

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

    # copy files from ./toolchain/lib to linaro_toolchain/lib
    message("Source Directory: ${CMAKE_SOURCE_DIR}")
    message("Project Source Directory: ${PROJECT_SOURCE_DIR}")

    # copy libstdc++ only once
    set(LIBSTDCPP_SRC "${CMAKE_SOURCE_DIR}/toolchain/lib/libstdc++.so.6.0.18")
    set(LIBSTDCPP_STAMP "${LINARO_DIR}/.libstdcpp_copied")
    if(EXISTS "${LIBSTDCPP_SRC}" AND NOT EXISTS "${LIBSTDCPP_STAMP}")
        file(COPY "${LIBSTDCPP_SRC}" DESTINATION "${LINARO_DIR}/arm-linux-gnueabi/libc/lib")
        file(COPY "${LIBSTDCPP_SRC}" DESTINATION "${LINARO_DIR}/arm-linux-gnueabi/lib")
        file(WRITE "${LIBSTDCPP_STAMP}" "copied\n")
        message(STATUS "libstdc++ copied to Linaro sysroot")

            #set symlinks
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libstdc++.so.6.0.18 "${LINARO_DIR}/arm-linux-gnueabi/libc/lib/libstdc++.so.6")
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libstdc++.so.6.0.18 "${LINARO_DIR}/arm-linux-gnueabi/libc/lib/libstdc++.so")
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libstdc++.so.6.0.18 "${LINARO_DIR}/arm-linux-gnueabi/lib/libstdc++.so.6")
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libstdc++.so.6.0.18 "${LINARO_DIR}/arm-linux-gnueabi/lib/libstdc++.so")

        message(STATUS "symlinks created")

    elseif(EXISTS "${LIBSTDCPP_STAMP}")
        message(STATUS "libstdc++ already copied; skipping")
        
    elseif(NOT EXISTS "${LIBSTDCPP_SRC}")
        message(WARNING "libstdc++ source not found at ${LIBSTDCPP_SRC}; skipping copy")
    endif()
    
    message(STATUS "copied libstdc++ to linario")

endif()

# Set compilers
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-g++)

# Set sysroot for cross-compilation
set(CMAKE_FIND_ROOT_PATH ${LINARO_DIR}/arm-linux-gnueabi/libc)
set(CMAKE_SYSROOT ${CMAKE_FIND_ROOT_PATH})

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

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DBUILD_TARGET_LINUX)
add_definitions(-DBUILD_TARGET_NEST)
