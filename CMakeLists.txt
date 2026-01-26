cmake_minimum_required(VERSION 3.16)
enable_testing()

# Use old linker behavior for compatibility with older toolchains
if(POLICY CMP0116)
    cmake_policy(SET CMP0116 OLD)
endif()

project(CuckooNest C CXX)

include(ExternalProject)
include(FetchContent)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Auto-detect and use ccache if available for faster rebuilds
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "ccache found and enabled for faster rebuilds")
endif()

# =====================================================
# Fetch json11 from public repository
# =====================================================
FetchContent_Declare(
    json11
    GIT_REPOSITORY https://github.com/dropbox/json11.git
    GIT_TAG master
    PATCH_COMMAND patch -p0 < ${CMAKE_SOURCE_DIR}/cmake/json11.patch
)
FetchContent_MakeAvailable(json11)

# Clang-tidy configuration
# Enabled by default for code quality, can be disabled with -DENABLE_CLANG_TIDY=OFF
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis during compilation" ON)

add_subdirectory("lvgl")
add_subdirectory("src")

# Enable clang-tidy for host builds only (if explicitly enabled or SKIP_CLANG_TIDY not set)
if(IS_HOST_BUILD AND (ENABLE_CLANG_TIDY OR NOT SKIP_CLANG_TIDY))
    find_program(CLANG_TIDY_EXE clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY 
            "${CLANG_TIDY_EXE}"
            "-checks=-*,readability-*,performance-*,clang-analyzer-*,modernize-use-auto,modernize-use-bool-literals,modernize-loop-convert,modernize-make-unique,modernize-make-shared,modernize-use-noexcept,modernize-use-override,modernize-use-nullptr,-readability-function-cognitive-complexity,-readability-magic-numbers,-readability-identifier-length,-modernize-use-trailing-return-type,-clang-analyzer-security.insecureAPI.strcpy,-google-*"
            "--warnings-as-errors=*"
        )
    else()
        message(WARNING "clang-tidy not found. Static analysis will be skipped.")
    endif()
else()
    if(IS_HOST_BUILD AND NOT ENABLE_CLANG_TIDY)
        message(STATUS "clang-tidy disabled. Re-enable with: cmake -DENABLE_CLANG_TIDY=ON")
    endif()
endif()

if(IS_HOST_BUILD)
add_subdirectory("tests")
endif()