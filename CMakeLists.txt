cmake_minimum_required(VERSION 3.20)
project(CuckooNest CXX)

# Option to build tests (defaults to OFF to maintain existing behavior)
option(BUILD_TESTS "Build unit tests" OFF)

# Option to build main application (defaults to ON)
option(BUILD_MAIN_APP "Build main application with cross-compiler" ON)

# If building tests, add tests subdirectory and exit early
#if(BUILD_TESTS)
    add_subdirectory(tests)
#    return()
#endif()

# The rest of this file is for cross-compiled main application
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
    src/ScreenManager.cpp
    src/HAL/Beeper.cpp
    src/HAL/Display.cpp
    src/HAL/BitmapFont.cpp
    src/HAL/Inputs.cpp
    src/Screens/HomeScreen.cpp
    src/Screens/DimmerScreen.cpp
    src/Screens/MenuScreen.cpp
)

add_executable(cuckoo ${SRC_FILES})

# Tell CMake that this target depends on the toolchain being ready
add_dependencies(cuckoo linaro_toolchain)

# Link threading library
target_link_libraries(cuckoo pthread)

# =====================================================
# 4. Set the cross-compiler for this project
# =====================================================
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${LINARO_BIN}/arm-linux-gnueabi-g++)
set(CMAKE_FIND_ROOT_PATH ${LINARO_DIR}/arm-linux-gnueabi/libc)
set(CMAKE_SYSROOT ${CMAKE_FIND_ROOT_PATH})

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Add flags, sysroot, etc.
target_compile_options(cuckoo PRIVATE
    --sysroot=${CMAKE_SYSROOT}
    -march=armv7-a
    -std=c++11
)

# =====================================================
# 5. Output location
# =====================================================
set_target_properties(cuckoo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output
)

