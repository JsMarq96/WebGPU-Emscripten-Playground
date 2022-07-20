#ifndef BASIC_RENDER_H_
#define BASIC_RENDER_H_

#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>


struct sRenderContext {
    WGPUSurface surface;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUSwapChain swapchain;
};

enum eShaderType : uint8_t {
    COMPUTE_SHADER = 0,
    RENDER_SHADER
};

struct sShader {
    eShaderType type;
    WGPUShaderModule module;

    const char* entrypoint_1 = NULL;
    const char* entryoiunt_2 = NULL;
};

#endif // BASIC_RENDER_H_
