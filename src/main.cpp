#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <cassert>
#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>
#include "basic_render.h"
#include "renderer_wgl2.h"

#define CANVAS_ID "render-canvas"

sRenderContext current_render_context;

sRenderer renderer;
WGPUInstance w_instance = NULL;



void frame_loop() {
    // Get current render target from the swapchain
    WGPUTextureView back_buffer = wgpuSwapChainGetCurrentTextureView(renderer.context.swapchain);
    //std::cout << "frame" << std::endl;
    //TODO renderWith(back_buffer, current_render_context);
    render(renderer, back_buffer);
}



void create_swapchain(const WGPUAdapter adapter,
                      const WGPUDevice device) {
    // Get & create the render surface
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas_descriptor = {}; // from the html
    canvas_descriptor.selector = "#canvas";
    canvas_descriptor.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    WGPUSurfaceDescriptor surface_descriptor = {.nextInChain = (WGPUChainedStruct*) &canvas_descriptor};
    WGPUSurface surface = wgpuInstanceCreateSurface(NULL, &surface_descriptor);

    // Define Swapchain
    WGPUSwapChainDescriptor swapchain_descriptor = {
      .usage = WGPUTextureUsage_RenderAttachment, // o output attachment??
      .format = WGPUTextureFormat_BGRA8Unorm,
      .width = 800, .height = 800, // TODO: fetch from canvas??
      .presentMode = WGPUPresentMode_Fifo
    };
    WGPUSwapChain swapchain = wgpuDeviceCreateSwapChain(device,
                                                        surface,
                                                        &swapchain_descriptor);

    // Store the current render context globally
    current_render_context = {
        .surface = surface,
        .adapter = adapter,
        .device = device,
        .queue = wgpuDeviceGetQueue(device),
        .swapchain = swapchain
    };

    renderer = create_renderer_with_context(current_render_context);
    emscripten_set_main_loop(frame_loop, 0, false);

}

void _device_callback(WGPURequestDeviceStatus status,
                      WGPUDevice device,
                      char const *message,
                      void *user_data) {
    //assert(status == WGPURequestDeviceStatus_Success && "Error fetching device");

    if (status != WGPURequestDeviceStatus_Success) {
        std::cout << "Could not load WebGPU, loading WebGL2" << std::endl;
        init_webgl2(CANVAS_ID);
    }

    WGPUAdapter *adapter = (WGPUAdapter*) user_data;
    // Application main
    std::cout << "Everything is a-ok" << std::endl;

    create_swapchain(*adapter, device);
}

void _adapter_callback(WGPURequestAdapterStatus status,
                       WGPUAdapter adapter,
                       char const *message,
                       void *userdata) {
    //assert(status == WGPURequestAdapterStatus_Success && "Error fetching adapter");
    if (status != WGPURequestAdapterStatus_Success) {
        std::cout << "Could not load WebGPU, loading WebGL2" << std::endl;
        init_webgl2(CANVAS_ID);
    }


    wgpuAdapterRequestDevice(adapter, NULL, _device_callback, (void*) &adapter);
}



inline void config_WGPU() {

    WGPUDevice device;

    wgpuInstanceRequestAdapter(w_instance, NULL, _adapter_callback, NULL);
}

inline void config_WebGL2() {

}

int EMSCRIPTEN_KEEPALIVE main() {
    std::cout << "Test!" << std::endl;

     config_WGPU();


    return 0;
}
