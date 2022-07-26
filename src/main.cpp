#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <cassert>
#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>
#include "basic_render.h"


sRenderContext current_render_context;


void frame_loop() {
    // Get current render target from the swapchain
    WGPUTextureView back_buffer = wgpuSwapChainGetCurrentTextureView(current_render_context.swapchain);

    //TODO renderWith(back_buffer, current_render_context);
}



void create_swapchain(const WGPUAdapter adapter,
                      const WGPUDevice device) {
    // Get & create the render surface
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas_descriptor = {.selector = "#render-canvas"}; // from the html
    WGPUSurfaceDescriptor surface_descriptor = {.nextInChain = (WGPUChainedStruct*) &canvas_descriptor};
    WGPUSurface surface = wgpuInstanceCreateSurface(NULL, &surface_descriptor);

    // Define Swapchain
    WGPUSwapChainDescriptor swapchain_descriptor = {
      .usage = WGPUTextureUsage_RenderAttachment,
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

    emscripten_set_main_loop(frame_loop, 0, false);

}

void _device_callback(WGPURequestDeviceStatus status,
                      WGPUDevice device,
                      char const *message,
                      void *user_data) {
    assert(status == WGPURequestDeviceStatus_Success && "Error fetching device");

    WGPUAdapter *adapter = (WGPUAdapter*) user_data;
    // Application main
    std::cout << "Everything is a-ok" << std::endl;
}

void _adapter_callback(WGPURequestAdapterStatus status,
                       WGPUAdapter adapter,
                       char const *message,
                       void *userdata) {
    assert(status == WGPURequestAdapterStatus_Success && "Error fetching adapter");

    wgpuAdapterRequestDevice(adapter, NULL, _device_callback, (void*) &adapter);
}



inline void config_WGPU() {

    WGPUDevice device;

    WGPUInstance w_instance = NULL;

    wgpuInstanceRequestAdapter(w_instance, NULL, _adapter_callback, NULL);
}

int EMSCRIPTEN_KEEPALIVE main() {
    std::cout << "Test!" << std::endl;

     config_WGPU();


    return 0;
}
