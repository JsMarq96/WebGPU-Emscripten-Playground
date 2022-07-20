#!/usr/bin/env bash
source ./emsdk/emsdk_env.sh

mkdir build/

cd build/

CMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
emcmake cmake ..

intercept-build make
cp compile_commands.json ../compile_commands.json
