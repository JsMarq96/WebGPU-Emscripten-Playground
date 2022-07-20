#include <cstdio>
#include <ctime>
#include <iostream>
#include <cassert>
#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>


void _device_callback(WGPURequestDeviceStatus status,
                      WGPUDevice,
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
