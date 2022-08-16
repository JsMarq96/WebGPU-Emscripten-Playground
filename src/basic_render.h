#ifndef BASIC_RENDER_H_
#define BASIC_RENDER_H_

#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

//#include <iostream>

#include "raw_shaders.h"

#define SHADER_COUNT 20
#define SHADER_ENTRYPOINT_LABEL_SIZE 20

struct sRenderContext {
    WGPUSurface surface;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSwapChain swapchain;
};

enum eShaderType : uint8_t {
    COMPUTE_SHADER = 0,
    RENDER_SHADER
};

struct sShader {
    eShaderType type;
    WGPUShaderModule module;

    const char entrypoint_1 [SHADER_ENTRYPOINT_LABEL_SIZE] = "";
    const char entrypoint_2 [SHADER_ENTRYPOINT_LABEL_SIZE] = "";
};


struct sRenderer {

    sRenderContext context;

    WGPURenderPassColorAttachment color_attachment;
    WGPURenderPassDepthStencilAttachment depth_attachment;
    WGPURenderPassDescriptor pass_descr;

    WGPURenderPipeline pipeline;

    // Buffers
    WGPUBuffer vertex_buffer;
    WGPUBuffer color_buffer;
    WGPUBuffer indices_buffer;

    // Create textures
    // Create render pipeline
    // COnfigure renderpass
    // On Frame:
    // 1 - Create Command encoder
    // 2 - Attach swapchain to teh render pass descriptor
    // 3 - attach render pass to encoder
    // 4 - Set the pipeline
    // 5 - Sert verticies
    // 6 - Draw command
    // 7 - Submit render queue

};

inline void upload_buffers_to_renderer(sRenderer *renderer) {
     // Raw data
    const float raw_vertices[] = {
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         -1.0f, -1.0f, 0.0f,    0.0, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,      0.0f, 0.0f, 1.0f
    };
    const uint16_t raw_indices[4] = { 1, 2, 3, 0 }; // Added a item for padding

    // Create & fill wgpu buffers

    { // Vertices
        WGPUBufferDescriptor buff_descr = {
               .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
               .size = sizeof(raw_vertices)
        };
        renderer->vertex_buffer = wgpuDeviceCreateBuffer(renderer->context.device,
                                                         &buff_descr);
        wgpuQueueWriteBuffer(renderer->context.queue,
                             renderer->vertex_buffer,
                             0,
                             raw_vertices,
                             sizeof(raw_vertices));
    }


    { // Indices
        WGPUBufferDescriptor buff_descr = {
               .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
               .size = sizeof(raw_indices)
        };
        renderer->indices_buffer = wgpuDeviceCreateBuffer(renderer->context.device,
                                                          &buff_descr);
        wgpuQueueWriteBuffer(renderer->context.queue,
                             renderer->indices_buffer,
                             0,
                             raw_indices,
                             sizeof(raw_indices));
    }

}

inline WGPUShaderModule create_shader_module(const sRenderer &renderer) {
    WGPUShaderModule shader;
    {
        WGPUShaderModuleWGSLDescriptor shader_wgsl_descr{};
        WGPUShaderModuleDescriptor shader_desc {};
        shader_wgsl_descr.source = raw_basic_shader;
        shader_wgsl_descr.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;

        shader_desc = (WGPUShaderModuleDescriptor){
           .nextInChain = (WGPUChainedStruct*)(&shader_wgsl_descr),
           .label = NULL,
        };

        shader = wgpuDeviceCreateShaderModule(renderer.context.device,
                                              &shader_desc);
    }

    return shader;
}

inline sRenderer create_renderer_with_context(sRenderContext &cont) {
    sRenderer renderer = {.context = cont};

    renderer.color_attachment = {
        .view = NULL,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.0f, 0.0f, 1.0f, 1.0f}
    };

    renderer.pass_descr = {
       .colorAttachmentCount = 1,
       .colorAttachments = &renderer.color_attachment,
       .depthStencilAttachment = NULL, // For now
    };

    return renderer;
}

#include <iostream>

inline void render(sRenderer &renderer, WGPUTextureView &text_view) {
    //renderer.color_attachment.view = text_view;

    renderer.color_attachment = {
        .view = NULL,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.0f, 0.0f, 1.0f, 1.0f}
    };

    renderer.pass_descr = {
       .colorAttachmentCount = 1,
       .colorAttachments = &renderer.color_attachment,
       .depthStencilAttachment = NULL, // For now
    };

    renderer.color_attachment.view = text_view;

    // Create a command encoder
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.context.device, NULL);
    WGPURenderPassEncoder r_pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderer.pass_descr);

    wgpuRenderPassEncoderEnd(r_pass);
    wgpuRenderPassEncoderRelease(r_pass);

    // Get the commands buffer
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, NULL);
    wgpuCommandEncoderRelease(encoder);

    // Submit to gpu
    wgpuQueueSubmit(renderer.context.queue, 1, &commands);

    wgpuCommandBufferRelease(commands);
   // wgpuTextureViewRelease(back_buffer);

    // NOTE: this is for DAWN/native
    //wgpuSwapChainPresent(renderer.context.swapchain);
}

#endif // BASIC_RENDER_H_
