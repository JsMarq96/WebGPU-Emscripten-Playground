#!/usr/bin/env bash

source ./emsdk/emsdk_env.sh

./build.sh

$EMSDK/upstream/emscripten/emrun --browser google-chrome-unstable build/MIX_WEBXR.html
