# Main application build configuration
# Use with ARM toolchain: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ..
# Use with host toolchain: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/host-toolchain.cmake ..

include(ExternalProject)
include(FetchContent)

# (removed spdlog FetchContent — using in-tree lightweight logger)

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
    Backplate/Message.cpp
    Backplate/CommandMessage.cpp
    Backplate/ResponseMessage.cpp
    Backplate/MessageParser.cpp
    Backplate/UnixSerialPort.cpp
    Backplate/BackplateComms.cpp
)

add_executable(cuckoo ${SRC_FILES})

# =====================================================
# Configure libcurl for cross-compilation
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

# Add project include directory (in-tree lightweight logger)
target_include_directories(cuckoo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# For runtime dynamic linking, we don't link libcurl at build time
# We use dlopen/dlsym to load it at runtime on the target device
target_link_libraries(cuckoo PRIVATE ${CMAKE_BINARY_DIR}/lvgl/src/lvgl-build/lib/liblvgl.a pthread dl)

# =====================================================
# Output location
# =====================================================
set_target_properties(cuckoo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output
)