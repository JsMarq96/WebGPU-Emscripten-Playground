#ifndef RENDERER_WGL2_H_
#define RENDERER_WGL2_H_

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl32.h>
#include <cassert>


inline EM_BOOL webgl2_render_loop(const double time, void *user_data) {

    return EM_TRUE;
}

inline void init_webgl2(const char* canvas_id) {
    EmscriptenWebGLContextAttributes wgl2_attrs = {};
    emscripten_webgl_init_context_attributes(&wgl2_attrs);
    wgl2_attrs.majorVersion = 2;

    // Create the web context on the canvas
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl = emscripten_webgl_create_context(canvas_id, &wgl2_attrs);

    assert(gl < 0 && "Error creating the context, webgl2 not available");

    emscripten_webgl_make_context_current(gl);

    emscripten_request_animation_frame_loop(webgl2_render_loop, NULL);
}

#endif // RENDERER_WGL2_H_
