cmake_minimum_required(VERSION 3.16)

set(LVGL_VERSION "v9.4.0")

set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lv_conf.h")

include(ExternalProject)

ExternalProject_Add(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG ${LVGL_VERSION}
    GIT_SHALLOW ON
    PREFIX ${CMAKE_BINARY_DIR}/lvgl
    CMAKE_ARGS
        -DLV_BUILD_CONF_PATH=${LV_CONF_PATH}
        -D CMAKE_C_STANDARD=99
        -D CMAKE_CXX_STANDARD=11
        -D CMAKE_C_COMPILER=${CMAKE_BINARY_DIR}/linaro/src/linaro_toolchain/bin/arm-linux-gnueabi-gcc
        -D CMAKE_CXX_COMPILER=${CMAKE_BINARY_DIR}/linaro/src/linaro_toolchain/bin/arm-linux-gnueabi-g++
        -D CMAKE_INSTALL_PREFIX=<BINARY_DIR>

)


