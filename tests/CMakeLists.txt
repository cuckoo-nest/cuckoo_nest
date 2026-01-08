cmake_minimum_required(VERSION 3.20)
project(CuckooTests CXX)

# Use the default system compiler for tests (not cross-compiler)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading GoogleTest
include(FetchContent)

# Download and configure GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#Define symbol to indicate unit tests are being built
add_definitions(-DUNIT_TESTS)

# Add tests directory for mock spdlog (BEFORE to take precedence)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

# Include directories for the source code
include_directories(../src)
include_directories(../include)
include_directories(../src/Screens)



# Define source files that need to be compiled for tests
# Exclude main.cpp as we don't want the main() function in tests
set(
    TEST_SOURCE_FILES
    ../src/ScreenManager.cpp
    ../src/ConfigurationReader.cpp
    ../third-party/json11/json11.cpp
    ../src/HAL/Beeper.cpp
    ../src/HAL/BitmapFont.cpp
    ../src/HAL/Inputs.cpp
    ../src/Integrations/IntegrationContainer.cpp
    ../src/Integrations/CurlWrapperJson.cpp
    ../src/Backplate/Message.cpp
    ../src/Backplate/CommandMessage.cpp
    ../src/Backplate/ResponseMessage.cpp
    ../src/Backplate/MessageParser.cpp
    ../src/Backplate/BackplateComms.cpp
)

# Define test files
set(
    TEST_FILES
    TestScreenManagerConfigLoad.cpp
    TestScreenManager.cpp
    TestConfigurationReader.cpp
    TestIntegrationContainer.cpp
    TestBackplateCommsMessage.cpp
    TestBackplateComms.cpp
    TestMessageParser.cpp
    TestCRCCITT.cpp
    ScreenStubs/DimmerScreen.cpp
    ScreenStubs/SwitchScreen.cpp
    ScreenStubs/MenuScreen.cpp
    ScreenStubs/HomeScreen.cpp
    ScreenStubs/AnalogClockScreen.cpp
)

# Create the test executable
add_executable(
    cuckoo_tests
    ${TEST_FILES}
    ${TEST_SOURCE_FILES}
)

# For host system tests, use system libcurl (not cross-compiled version)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Add libcurl include directories for host compilation
target_include_directories(cuckoo_tests PRIVATE ${CURL_INCLUDE_DIRS})

# Add json11 include directory for tests
target_include_directories(cuckoo_tests PRIVATE ../third-party/json11)
target_include_directories(cuckoo_tests PRIVATE ${CMAKE_BINARY_DIR}/lvgl/src)

target_link_directories(cuckoo_tests PRIVATE ${CMAKE_BINARY_DIR}/lvgl/src/lvgl-build/lib)

# Link against GoogleTest libraries (including GMock) and system libcurl
target_link_libraries(
    cuckoo_tests
    gtest_main
    gtest
    gmock
    gmock_main
    pthread
    ${CURL_LIBRARIES}
    liblvgl.a
)

# Enable testing
enable_testing()

# Include GoogleTest's CMake integration
include(GoogleTest)
gtest_discover_tests(cuckoo_tests)