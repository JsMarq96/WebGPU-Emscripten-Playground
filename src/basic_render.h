#ifndef BASIC_RENDER_H_
#define BASIC_RENDER_H_

#ifdef __EMSCRIPTEM__
#include <emscripten/emscripten.h>
#endif
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
    WGPUTexture depth_texture;
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
         1.0f, 0.0f, -1.0f,     1.0f, 0.0f, 0.0f,
         -1.0f, 0.0f, -1.0f,    0.0, 1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f
    };
    const uint16_t raw_indices[4] = { 0, 1, 2, 0 }; // Added a item for padding

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

inline void create_depth_attachment(sRenderer *renderer) {
    WGPUTextureDescriptor descr = {
       .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc,
       .dimension = WGPUTextureDimension_2D, // A 2D texture ??
       .size = {.width = 400, .height = 400, .depthOrArrayLayers = 1},
       .format = WGPUTextureFormat_Depth16Unorm,
       .mipLevelCount = 1,
       .sampleCount = 1,
    };

    renderer->depth_texture = wgpuDeviceCreateTexture(renderer->context.device, &descr);

    WGPUTextureView depth_tex_view = wgpuTextureCreateView(renderer->depth_texture, NULL);

    renderer->depth_attachment = {
       .view = depth_tex_view,
       .depthLoadOp = WGPULoadOp_Clear,
       .depthStoreOp = WGPUStoreOp_Store,
       .depthClearValue = 1.0f
    };
}

inline sRenderer create_renderer_with_context(sRenderContext &cont) {
    sRenderer renderer = {.context = cont};

    upload_buffers_to_renderer(&renderer);
    create_depth_attachment(&renderer);

    WGPUShaderModule shader = create_shader_module(renderer);

    // Render Pipeline
    WGPUPipelineLayoutDescriptor pipeline_descr = {
         .bindGroupLayoutCount = 0,
         .bindGroupLayouts = NULL
    };

    WGPURenderPipelineDescriptor descr = { .layout = wgpuDeviceCreatePipelineLayout(renderer.context.device, &pipeline_descr)};

    WGPUColorTargetState color_state = {.format = WGPUTextureFormat_BGRA8Unorm};
    WGPUDepthStencilState depth_state = {.format = WGPUTextureFormat_Depth16Unorm, .depthWriteEnabled = false};

    WGPUVertexAttribute atributes[2] = {
        {
          .format = WGPUVertexFormat_Float32x3,
          .offset = 0,
          .shaderLocation = 0
        },
        {
          .format = WGPUVertexFormat_Float32x3,
          .offset = sizeof(float) * 3,
          .shaderLocation = 1
        }
    };
    WGPUVertexBufferLayout vertex_layout = {
        .arrayStride = sizeof(float) * 6,
        .stepMode = WGPUVertexStepMode_Vertex,
        .attributeCount = 2,
        .attributes = atributes
    };
    descr.vertex = {
        .module = shader,
        .entryPoint = "vert_main",
        .bufferCount = 1,
        .buffers = &vertex_layout
    };
    WGPUFragmentState frag_state = {
         .module = shader,
         .entryPoint = "frag_main",
         .targetCount = 1,
         .targets = &color_state,
    };

    descr.fragment = &frag_state;
    descr.depthStencil = &depth_state;
    descr.multisample.count = 1;
    // Primitve state: define the face culling & primitive for the pipeline
    descr.primitive.frontFace = WGPUFrontFace_CCW;
    descr.primitive.cullMode = WGPUCullMode_None;
    descr.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    descr.primitive.stripIndexFormat = WGPUIndexFormat_Undefined; // ??

    renderer.pipeline = wgpuDeviceCreateRenderPipeline(renderer.context.device, &descr);

    return renderer;
}

#include <iostream>

inline void render(sRenderer &renderer, WGPUTextureView &text_view) {
    renderer.color_attachment = {
        .view = text_view,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    renderer.pass_descr = {
       .colorAttachmentCount = 1,
       .colorAttachments = &renderer.color_attachment,
       .depthStencilAttachment = &renderer.depth_attachment,
    };


    // Create a command encoder
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.context.device, NULL);
    WGPURenderPassEncoder r_pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderer.pass_descr);

    // Define the render pass
    wgpuRenderPassEncoderSetPipeline(r_pass, renderer.pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(r_pass, 0, renderer.vertex_buffer, 0, WGPU_WHOLE_SIZE);
    wgpuRenderPassEncoderDraw(r_pass, 3, 1, 0, 0);

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
