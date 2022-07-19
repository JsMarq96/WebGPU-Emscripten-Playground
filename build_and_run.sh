#!/usr/bin/env bash

./build.sh

emscripten/emrun --browser chrome build/MIX_WEBXR.html
