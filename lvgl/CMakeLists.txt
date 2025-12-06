cmake_minimum_required(VERSION 3.16)

set(LVGL_VERSION "v9.4.0")

set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lv_conf.h")

include(ExternalProject)

# Prepare compiler flags for LVGL build
set(LVGL_C_FLAGS "${CMAKE_C_FLAGS_INIT}")
set(LVGL_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT}")

# Pass HOST_TOOLCHAIN definition to LVGL if we're building for host
if(IS_HOST_BUILD)
    set(LVGL_C_FLAGS "${LVGL_C_FLAGS} -DHOST_TOOLCHAIN")
    set(LVGL_CXX_FLAGS "${LVGL_CXX_FLAGS} -DHOST_TOOLCHAIN")
    message(STATUS "LVGL will be built with SDL support for host platform")
endif()

ExternalProject_Add(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG ${LVGL_VERSION}
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    PREFIX ${CMAKE_BINARY_DIR}/lvgl
    CMAKE_ARGS
        -DLV_BUILD_CONF_PATH=${LV_CONF_PATH}
        -DCMAKE_C_STANDARD=99
        -DCMAKE_CXX_STANDARD=11
        -D CONFIG_LV_BUILD_DEMOS=OFF
        -D CONFIG_LV_BUILD_EXAMPLES=OFF
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
        -DCMAKE_C_FLAGS=${LVGL_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${LVGL_CXX_FLAGS}
        -DCMAKE_INSTALL_PREFIX=<BINARY_DIR>

)


