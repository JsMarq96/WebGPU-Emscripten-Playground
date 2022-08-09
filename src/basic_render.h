#ifndef BASIC_RENDER_H_
#define BASIC_RENDER_H_

#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>

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

    WGPUTextureView color_attachment;
    WGPUTextureView depth_attachment;

    WGPURenderPipeline pipeline;

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

inline sRenderer create_renderer_with_context(sRenderContext &cont) {
    sRenderer renderer = {.context = cont};
    // 1 - Create render targets
    // Color
    {
       renderer.color_attachment = wgpuSwapChainGetCurrentTextureView(renderer.context.swapchain);
    }
    // Depth
    {
        WGPUTextureDescriptor descr_color_tex = {
          .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc,
          .size = {200, 200, 1},
          .format = WGPUTextureFormat_BGRA8Unorm,
          .sampleCount = 1
        };
        WGPUTexture color_tex = wgpuDeviceCreateTexture(renderer.context.device, NULL);
        renderer.color_attachment = wgpuTextureCreateView(color_tex, NULL);
    }

    // 2 - Create Buffers
    // Raw data
    const float raw_vertices[] = {1.0f, -1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    const float raw_colors[] = { 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f };
    const uint16_t raw_indices[] = { 1, 2, 3 };

    // Create & fill wgpu buffers
    WGPUBuffer vertex_buffer;
    WGPUBuffer color_buffer;
    WGPUBuffer indices_buffer;


    { // Vertices
        WGPUBufferDescriptor buff_descr = {
               .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
               .size = sizeof(raw_vertices)
        };
        vertex_buffer = wgpuDeviceCreateBuffer(renderer.context.device, &buff_descr);
        wgpuQueueWriteBuffer(renderer.context.queue,
                             vertex_buffer,
                             0,
                             raw_vertices,
                             sizeof(raw_vertices));
    }

    { // Colors
        WGPUBufferDescriptor buff_descr = {
               .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
               .size = sizeof(raw_colors)
        };
        color_buffer = wgpuDeviceCreateBuffer(renderer.context.device, &buff_descr);
        wgpuQueueWriteBuffer(renderer.context.queue,
                             color_buffer,
                             0,
                             raw_colors,
                             sizeof(raw_colors));
    }

    { // Indices
        WGPUBufferDescriptor buff_descr = {
               .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
               .size = sizeof(raw_indices)
        };
        indices_buffer = wgpuDeviceCreateBuffer(renderer.context.device, &buff_descr);
        wgpuQueueWriteBuffer(renderer.context.queue,
                             indices_buffer,
                             0,
                             raw_indices,
                             sizeof(raw_indices));
    }

    // 3 - Create shaders
    WGPUShaderModule basic_shader;
    {
        WGPUShaderModuleWGSLDescriptor shader_wgsl_descr{};
        WGPUShaderModuleDescriptor shader_desc {};
        shader_wgsl_descr.source = raw_basic_shader;
        shader_wgsl_descr.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;

        shader_desc.nextInChain = (WGPUChainedStruct*) &shader_desc;
        shader_desc.label = NULL;

        basic_shader = wgpuDeviceCreateShaderModule(renderer.context.device,
                                                    &shader_desc);
    }

    // 4 - Crete the rendering pipeline
    // Bindgroup layout: EMPTY for this test
    WGPUBindGroup bind_group;
    {
        // The layout is composed on entries
        //   each entry defines the visibility of the buffer,
        //   the buffer, adn the biding position
        WGPUBindGroupLayoutDescriptor layout_descr = {};
        WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(renderer.context.device,
                                                                     &layout_descr);

        WGPUBindGroupDescriptor desc = {  .layout = layout, .entryCount = 0, .entries = NULL };
        bind_group = wgpuDeviceCreateBindGroup(renderer.context.device, &desc);
    }

    // Create pipeline
    {
        WGPUPipelineLayoutDescriptor pipeline_descr = {.bindGroupLayoutCount = 0, .bindGroupLayouts = NULL };

        WGPURenderPipelineDescriptor descr = { .layout = wgpuDeviceCreatePipelineLayout(renderer.context.device, &pipeline_descr)};
        // Shader states
        // - Color state: define for the color & blending stages of the pipeline
        WGPUColorTargetState color_state = {.format = WGPUTextureFormat_BGRA8Unorm };
        // - Depth stencil: define the depth ofr the pass
        // None needed on this example
        // - Vertex state: define the vertex shader
        descr.vertex = {.module = basic_shader, .entryPoint = "vert_main"};
        // - Fragemtn state: idem
        WGPUFragmentState frag_state = {.module = basic_shader, .entryPoint = "frag_main", .targetCount = 1, .targets = &color_state};
        descr.fragment = &frag_state;
        // - Primitve state: define the face culling & primitive for the pipeline
        descr.primitive.frontFace = WGPUFrontFace_CCW;
        descr.primitive.cullMode = WGPUCullMode_None;
        descr.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        descr.primitive.stripIndexFormat = WGPUIndexFormat_Undefined; // ??

        renderer.pipeline = wgpuDeviceCreateRenderPipeline(renderer.context.device, &descr);
    }

    return renderer;
}

inline void render(const sRenderer &renderer, WGPUTextureView &text_view) {
    WGPURenderPassColorAttachment attachment = { .view = text_view, .loadOp = WGPULoadOp_Clear, .storeOp = WGPUStoreOp_Store, .clearValue= {0.0f, 0.0f, 0.0f, 1.0f} };
    WGPURenderPassDescriptor renderpass = { .colorAttachmentCount = 1, .colorAttachments = &attachment };

    WGPUCommandBuffer commands;
    {
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.context.device, NULL);
        {
            WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, NULL);

            wgpuRenderPassEncoderSetPipeline(pass, renderer.pipeline);
            wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
            wgpuRenderPassEncoderEnd(pass);
        }
        wgpuCommandEncoderFinish(encoder, NULL);
    }
    wgpuQueueSubmit(renderer.context.queue, 1, &commands);
}

#endif // BASIC_RENDER_H_
