#!/usr/bin/env bash

source ./emsdk/emsdk_env.sh

./build.sh

$EMSDK/upstream/emscripten/emrun build/MIX_WEBXR.html --browser google-chrome-unstable

#google-chrome-unstable build/MIX_WEBXR.html --enable-unsafe-wegbpu
