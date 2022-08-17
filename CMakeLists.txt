cmake_minimum_required(VERSION 3.13)

project(MIX_WEBXR)

file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB CPP_SUBFOLDER_SOURCES "src/**/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB C_SUBFOLDER_SOURCES "src/**/*.c")

if(NOT EMSCRIPTEN)
  #Use Dawn
  set(DAWN_ENABLE_PIC        ON CACHE BOOL "Position-Independent-Code")
  set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "OpenGL backend")
  set(DAWN_ENABLE_OPENGLES   OFF CACHE BOOL "OpenGL ES backend")
  set(DAWN_BUILD_EXAMPLES    OFF CACHE BOOL "Dawn examples")
  set(TINT_BUILD_SAMPLES     OFF CACHE BOOL "Tint examples")
  set(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "OpenGL SL writer")
  add_subdirectory("external/dawn" EXCLUDE_FROM_ALL)

else()
  file(GLOB EMS_INCLUDES_SUB "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*/*.h")
  file(GLOB EMS_INCLUDES "$ENV{EMSDK}/upstream/emscripten/cache/sysroot/include/*.h")

  include_directories(MIX_WEBXR ${EMS_INCLUDES} ${EMS_INCLUDES_SUB})
endif()

add_executable(MIX_WEBXR ${CPP_SOURCES} ${C_SOURCES})

message(STATUS "Exporting with WASM")

if (EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set_target_properties(MIX_WEBXR PROPERTIES LINK_FLAGS "-s WASM=1 -s USE_WEBGPU=1 -s STRICT=1 -lGL --shell-file ../web/page.html")
else()
  target_link_libraries(MIX_WEBXR
    dawn_headers
    dawncpp
    dawn_native
    dawn_proc
    glfw)
endif()
