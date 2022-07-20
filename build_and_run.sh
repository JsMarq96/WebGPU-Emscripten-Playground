#!/usr/bin/env bash

./build.sh

$EMSDK/upstream/emscripten/emrun --browser google-chrome-unstable build/MIX_WEBXR.html
