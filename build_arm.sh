#!/bin/bash
mkdir -p ./build_arm
cmake -B ./build_arm -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake
cmake --build ./build_arm/