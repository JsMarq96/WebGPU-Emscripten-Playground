cmake_minimum_required(VERSION 3.13)

project(MIX_WEBXR)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")

add_executable(MIX_WEBXR ${CPP_SOURCES} ${C_SOURCES})

message(STATUS "Exporting with WASM")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set_target_properties(MIX_WEBXR PROPERTIES LINK_FLAGS "-s WASM=1 -s TOTAL_MEMORY=67108864 -s EXPORT_ALL=1 -s FORCE_FILESYSTEM=1 -s FETCH -s EXPORTED_RUNTIME_METHODS=['ccall'] --shell-file ../web/page.html")
