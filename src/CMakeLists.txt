# Cross-compiled main application build configuration
# This builds the ARM executable for the embedded target

include(ExternalProject)

# Disable dependency file generation for older toolchains
set(CMAKE_CXX_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_C_DEPENDS_USE_COMPILER FALSE)

# =====================================================
# 3. Define your actual build target
# =====================================================
set (
    SRC_FILES 
    main.cpp
    ScreenManager.cpp
    ConfigurationReader.cpp
    ../third-party/json11/json11.cpp
    HAL/Beeper.cpp
    HAL/Display.cpp
    HAL/BitmapFont.cpp
    HAL/Inputs.cpp
    HAL/Backlight.cpp
    Screens/HomeScreen.cpp
    Screens/DimmerScreen.cpp
    Screens/MenuScreen.cpp
    Screens/SwitchScreen.cpp
    Integrations/IntegrationContainer.cpp
    Screens/CuckooLogoNest.cpp
)

add_executable(cuckoo ${SRC_FILES})

# Tell CMake that this target depends on the toolchain being ready
add_dependencies(cuckoo linaro_toolchain)

# =====================================================
# 4. Configure libcurl for cross-compilation
# =====================================================
# Use the libcurl headers we've copied to the project for cross-compilation
set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third-party/libcurl/include")

if(EXISTS ${CURL_INCLUDE_DIR}/curl/curl.h)
    target_include_directories(cuckoo PRIVATE ${CURL_INCLUDE_DIR})
    message(STATUS "Using project libcurl headers at: ${CURL_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Could not find libcurl headers in ${CURL_INCLUDE_DIR}")
endif()

# Add json11 include directory
target_include_directories(cuckoo PRIVATE ${CMAKE_SOURCE_DIR}/third-party/json11)
target_include_directories(cuckoo PRIVATE ${CMAKE_BINARY_DIR}/lvgl/src)

# For runtime dynamic linking, we don't link libcurl at build time
# We use dlopen/dlsym to load it at runtime on the target device
target_link_libraries(cuckoo PRIVATE ${CMAKE_BINARY_DIR}/lvgl/src/lvgl-build/lib/liblvgl.a pthread dl)

# =====================================================
# 5. Set the cross-compiler for this project
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
# 6. Output location
# =====================================================
set_target_properties(cuckoo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output
)