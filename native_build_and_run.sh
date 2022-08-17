#!/usr/bin/env bash

mkdir build
cd build/
cmake ..

intercept-build make -j 8
./MIX_WEBXR
