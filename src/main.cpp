#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <cassert>
#include "basic_render.h"

#ifdef __EMSCRIPTEN__
// Load Emscripten libs
#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>
#include <emscripten/html5_webgpu.h>
#define CANVAS_ID "render-canvas"

#else
// Load Dawn libs
#include <dawn/webgpu.h>
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <GLFW/glfw3.h>

#endif

sRenderContext current_render_context;

sRenderer renderer;
WGPUInstance w_instance = NULL;



void frame_loop() {
    // Get current render target from the swapchain
    WGPUTextureView back_buffer = wgpuSwapChainGetCurrentTextureView(renderer.context.swapchain);
    //std::cout << "frame" << std::endl;
    //TODO renderWith(back_buffer, current_render_context);
    render(renderer, back_buffer);

    wgpuTextureViewRelease(back_buffer);
}


#ifdef __EMSCRIPTEN__
// =======================
// EMSCRIPTEN LOADER ---------------
// =======================

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
      .width = 400, .height = 400, // TODO: fetch from canvas??
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

    std::cout << "Everything is a-ok" << std::endl;
    emscripten_set_main_loop(frame_loop, 0, false);
}

void _device_callback(WGPURequestDeviceStatus status,
                      WGPUDevice device,
                      char const *message,
                      void *user_data) {
    //assert(status == WGPURequestDeviceStatus_Success && "Error fetching device");

    if (status != WGPURequestDeviceStatus_Success) {
        std::cout << "Could not load WebGPU, loading WebGL2" << std::endl;
        //init_webgl2(CANVAS_ID);
    }

    WGPUAdapter *adapter = (WGPUAdapter*) user_data;
    // Application main

    create_swapchain(*adapter, device);
}

void _adapter_callback(WGPURequestAdapterStatus status,
                       WGPUAdapter adapter,
                       char const *message,
                       void *userdata) {
    //assert(status == WGPURequestAdapterStatus_Success && "Error fetching adapter");
    if (status != WGPURequestAdapterStatus_Success) {
        std::cout << "Could not load WebGPU, loading WebGL2" << std::endl;
        //init_webgl2(CANVAS_ID);
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

// =======================
// EMSCRIPTEN LOADER  END---------------
// =======================
#else
dawn_native::Instance instance;


dawn_native::Adapter fetch_preffered_adapter(const std::vector<dawn_native::Adapter>& adapters) {
    wgpu::AdapterType prefered_hardware[] = {
       wgpu::AdapterType::DiscreteGPU,
       wgpu::AdapterType::IntegratedGPU
    };
    wgpu::BackendType preferred_backend = wgpu::BackendType::Vulkan;

    for(uint8_t i = 0; i < 2; i++) {
        for(uint8_t j = 0; j < adapters.size(); j++) {
            wgpu::AdapterProperties props = {};
            adapters[j].GetProperties(&props);

            if (props.adapterType == prefered_hardware[i] && props.backendType == preferred_backend) {
                return adapters[j];
            }
        }
    }

    return {};
}

void GetDevice() {
    instance = dawn_native::Instance();
    instance.DiscoverDefaultAdapters();

    // Get an adapter for the backend to use, and create the device.
    dawn_native::Adapter backendAdapter = fetch_preffered_adapter(instance.GetAdapters());
    //assert(backendAdapter.GetBackendType() == dawn_native::BackendType::Metal);

    // Feature toggles
    wgpu::DawnTogglesDeviceDescriptor feature_toggles{};
    std::vector<const char*> enabled_toggles;
    enabled_toggles.push_back("disallow_spirv"); // Prevents SPIR-V since it doesnt work well on apple

    feature_toggles.forceEnabledTogglesCount = enabled_toggles.size();
    feature_toggles.forceEnabledToggles = &enabled_toggles[0];

    wgpu::DeviceDescriptor descr = {
       .nextInChain = &feature_toggles,
       .label = "DawnDevice"
    };

    wgpu::Device device = wgpu::Device::Acquire(backendAdapter.CreateDevice(&descr));
    DawnProcTable procs = dawn_native::GetProcs();

    //dawnProcSetProcs(&procs);
    //callback(device);
}

void create_window() {
    if (!glfwInit()) {
        return;
        //return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 800, "WIN_NAME", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!window) {
        std::cout << "Error, could not create window" << std::endl;
    } else {
        // Create webgpu context
        GetDevice();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
#endif
