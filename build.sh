#!/usr/bin/env bash

variant="${1:=host}"
builddir="./build_${variant}"
cmaketarget="../cmake/${variant}-toolchain.cmake"

test ! -d "${builddir}" && mkdir -p "${builddir}"
test "-f ${cmaketarget}" -a ! -f "${builddir}/CMakeCache.txt" && cmake -B "${builddir}" "-DCMAKE_TOOLCHAIN_FILE=${cmaketarget}"
test -d "${builddir}" -a -f "${builddir}/CMakeCache.txt" && cmake --build "${builddir}"