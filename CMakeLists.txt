cmake_minimum_required(VERSION 3.13)

project(MIX_WEBXR)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")
file(GLOB EMS_INCLUDES_SUB "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*/*.h")
file(GLOB EMS_INCLUDES "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*.h")

include_directories(MIX_WEBXR ${EMS_INCLUDES} ${EMS_INCLUDES_SUB})

add_executable(MIX_WEBXR ${CPP_SOURCES} ${C_SOURCES})

message(STATUS "Exporting with WASM")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set_target_properties(MIX_WEBXR PROPERTIES LINK_FLAGS "-s WASM=1 -s USE_WEBGL2=1 -s USE_WEBGPU=1 -s STRICT=1 -lGL --shell-file ../web/page.html")
