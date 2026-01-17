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


add_subdirectory("lvgl")
add_subdirectory("src")

# Enable clang-tidy for host builds only
if(IS_HOST_BUILD AND NOT SKIP_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY 
            "${CLANG_TIDY_EXE}"
            "-checks=-*,readability-*,performance-*,clang-analyzer-*,modernize-use-auto,modernize-use-bool-literals,modernize-loop-convert,modernize-make-unique,modernize-make-shared,modernize-use-noexcept,modernize-use-override,modernize-use-nullptr,-readability-function-cognitive-complexity,-readability-magic-numbers,-readability-identifier-length,-modernize-use-trailing-return-type,-clang-analyzer-security.insecureAPI.strcpy,-google-*"
            "--warnings-as-errors=*"
            #"--header-filter: '^(?!.*/build.*/lvgl|.*/json11).*(.*\.(hpp?|cpp))$'"
        )
    else()
        message(WARNING "clang-tidy not found. Static analysis will be skipped.")
    endif()
endif()

if(IS_HOST_BUILD)
add_subdirectory("tests")
endif()