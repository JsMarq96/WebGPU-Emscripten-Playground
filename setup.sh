#!/usr/bin/env bash
emsdk/emsdk update
emsdk/emsdk install latest
emsdk/emsdk activate latest

source ./emsdk/emsdk_env.sh
