#!/usr/bin/env bash

mkdir build/

cd build/

CMAKE_TOOLCHAIN_FILE=../emscripten/cmake/Modules/Platform/Emscripten.cmake
../emscripten/emcmake cmake ..

intercept-build make
cp compile_commands.json ../compile_commands.json
