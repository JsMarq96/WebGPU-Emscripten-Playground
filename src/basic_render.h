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

    WGPUTextureView color_attachment;
    WGPUTextureView depth_attachment;

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

    upload_buffers_to_renderer(&renderer);

    // 1 - Create render targets
    {
        // Create a Depth Texture
        WGPUTextureDescriptor descr = {
            .usage = WGPUTextureUsage_RenderAttachment,
            .size = { .width = 800, .height = 800, .depthOrArrayLayers = 1},
            .format = WGPUTextureFormat_Depth16Unorm,
        };

        WGPUTexture depth = wgpuDeviceCreateTexture(renderer.context.device, &descr);
        renderer.depth_attachment = wgpuTextureCreateView(depth, NULL);
    }

    // 3 - Create shaders
    WGPUShaderModule shader = create_shader_module(renderer);

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
        WGPUPipelineLayoutDescriptor pipeline_descr = {
           .bindGroupLayoutCount = 0,
           .bindGroupLayouts = NULL
        };
        WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(renderer.context.device,
                                                                   &pipeline_descr);

        // GEOMETRY BUFFER LAYOUT
        // Geometry position
        WGPUVertexAttribute atributes[2] = {};
        // Position attrribute
        atributes[0] = {
           .format = WGPUVertexFormat_Float32x3,
           .offset = 0,
           .shaderLocation = 0
        };
        // Color attribute
        atributes[1] =  {
           .format = WGPUVertexFormat_Float32x3,
           .offset = sizeof(float) * 3,
           .shaderLocation = 1
        };

        WGPUVertexBufferLayout buff_layout = {
          .arrayStride = 6 * sizeof(float),
          .attributeCount = 2,
          .attributes = atributes
        };


        // Color state: define for the color & blending stages of the pipeline
        WGPUColorTargetState color_state = {.format = WGPUTextureFormat_BGRA8Unorm };
        WGPUFragmentState frag_state = {
           .module = shader,
           .entryPoint = "frag_main",
           .targetCount = 1,
           .targets = &color_state
        };


        // Pipeline
       // WGPURenderPipelineDescriptor descr = {
           // .layout = wgpuDeviceCreatePipelineLayout(renderer.context.device, &pipeline_descr)
        //};

         WGPURenderPipelineDescriptor descr = {
           .label = NULL,
           .layout = layout,
           .vertex = {
              .module = shader,
              .entryPoint = "vert_main",
              .bufferCount = 1,
              .buffers = &buff_layout
           },
           .primitive = {
               .topology = WGPUPrimitiveTopology_TriangleList,
               .stripIndexFormat = WGPUIndexFormat_Undefined, // ??
               .frontFace = WGPUFrontFace_CCW,
               .cullMode = WGPUCullMode_None
           },
           .fragment = &frag_state,
        };

        renderer.pipeline = wgpuDeviceCreateRenderPipeline(renderer.context.device, &descr);
    }

    return renderer;
}

#include <iostream>

inline void render(sRenderer &renderer, WGPUTextureView &text_view) {
    WGPUTextureView back_buffer = wgpuSwapChainGetCurrentTextureView(renderer.context.swapchain);

    WGPURenderPassColorAttachment attachment = {
        .view = NULL,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {1.0f, 1.0f, 0.0f, 1.0f}
    };

     WGPURenderPassDepthStencilAttachment depth_attach = {
        .view = renderer.depth_attachment,
        .depthLoadOp = WGPULoadOp_Clear,
        .depthStoreOp = WGPUStoreOp_Store,
        .depthClearValue = 1.0f,
     };

    WGPURenderPassDescriptor renderpass_descr = {
         .colorAttachmentCount = 1,
         .colorAttachments = &attachment,
         .depthStencilAttachment = &depth_attach,
    };

    WGPUBuffer buf = renderer.vertex_buffer;
    WGPUCommandBuffer commands;
    {
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(renderer.context.device, NULL);
        {
            WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderpass_descr);
            wgpuRenderPassEncoderSetPipeline(pass,
                                             renderer.pipeline);
            wgpuRenderPassEncoderSetVertexBuffer(pass, 0, renderer.vertex_buffer, 0, WGPU_WHOLE_SIZE);
            wgpuRenderPassEncoderDraw(pass, 3, 0, 0, 0);
            //wgpuRenderPassEncoderSetIndexBuffer(pass, renderer.indices_buffer, WGPUIndexFormat_Uint16, 0, WGPU_WHOLE_SIZE);
            //wgpuRenderPassEncoderDrawIndexed(pass, 3, 1, 0, 0, 0);
            wgpuRenderPassEncoderEnd(pass);
            wgpuRenderPassEncoderRelease(pass);
        }
        commands = wgpuCommandEncoderFinish(encoder, NULL);
        wgpuCommandEncoderRelease(encoder);
    }
    wgpuQueueSubmit(renderer.context.queue, 1, &commands);
    wgpuCommandBufferRelease(commands);

    //wgpuTextureViewRelease(back_buffer);
}

#endif // BASIC_RENDER_H_
