#include <gfx/gfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "core/result.h"
#include "vk.h"

struct GfxDevice_T {
    VkContext                   ctx;
    Array(VkDeviceMemory)       device_memory;
    GraphicsPipeline            graphics_pipeline;
    Swapchain                   swapchain;
    FrameData                   frames;
    VkCommandPool               command_pool;
    VkRenderPass                render_pass;
    MemoryPool                  vertex_buffers;
};

DECLARE_ARRAY(GfxVertexLayout_T);

struct GfxPipelineDesc {
    const char                  *vertex_shader_path;
    const char                  *fragment_shader_path;
    bool                        depth_Test;
    Array(GfxVertexLayout_T)    vertex_desc;
};

typedef struct {
    const char **validation_layers;
    uint32_t layers_count;
    GfxWindow *window;
} VkContextCreateInfo;

void gfx_graphics_loader_version_log(void) {
    uint32_t inst;
    vkEnumerateInstanceVersion(&inst);
    printf("Vulkan loader version: %u\n", inst);
}

void gfx_graphics_api_info_log(GfxDevice_T *dev) {
    VkPhysicalDeviceProperties p;
    vkGetPhysicalDeviceProperties(dev->ctx.physical_device, &p);

    printf("GPU vulkan version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(p.apiVersion),
        VK_API_VERSION_MINOR(p.apiVersion),
        VK_API_VERSION_PATCH(p.apiVersion)
    );
}

struct GfxPipeline {
    GraphicsPipeline pipeline;
};

// gfx_vulkan.c
static VkFormat gfx_format_to_vk(GfxFormat f) {
    switch (f) {
        case GFX_FORMAT_FLOAT:       return VK_FORMAT_R32_SFLOAT;
        case GFX_FORMAT_FLOAT2:      return VK_FORMAT_R32G32_SFLOAT;
        case GFX_FORMAT_FLOAT3:      return VK_FORMAT_R32G32B32_SFLOAT;
        case GFX_FORMAT_FLOAT4:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GFX_FORMAT_UBYTE4_NORM: return VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_FORMAT_UNDEFINED;
}

Result
gfx_pipeline_create(GfxDevice_T *dev, GfxPipelineDesc *desc) {
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;

    TRY(vulkan_shader_module_create_from_file(dev->ctx.logical_device, "obj/shaders/shader.vert.spv", &vert_shader));
    TRY(vulkan_shader_module_create_from_file(dev->ctx.logical_device, "obj/shaders/shader.frag.spv", &frag_shader));

    VkVertexInputBindingDescription bind_desc[GFX_MAX_BINDINGS] = {0};
    VkVertexInputAttributeDescription attr_desc[GFX_MAX_BINDINGS * GFX_MAX_ATTRIBUTES] = {0};
    uint32_t attr_count = 0;

    for (size_t i = 0; i < desc->vertex_layout.binding_count; i++) {
        GfxVertexBinding *binding = &desc->vertex_layout.bindings[i];
        bind_desc[i] = (VkVertexInputBindingDescription) {
            .binding = i,
            .stride = binding->stride,
            .inputRate = binding->per_instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX,
        };
        for (size_t j = 0; j < binding->attribute_count; j++) {
            GfxVertexAttribute *attr = &binding->attributes[j];
            attr_desc[attr_count++] = (VkVertexInputAttributeDescription) {
                .binding = i,
                .format = gfx_format_to_vk(attr->format),
                .offset = attr->offset,
                .location = attr->location
            };
        }
    }

    VkPipelineShaderStageCreateInfo vert_stage_info;
    memset(&vert_stage_info, 0, sizeof(vert_stage_info));
    vert_stage_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage               = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module              = vert_shader;
    vert_stage_info.pName               = "main";
    vert_stage_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo frag_stage_info;
    memset(&frag_stage_info, 0, sizeof(frag_stage_info));
    frag_stage_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module              = frag_shader;
    frag_stage_info.pName               = "main";
    frag_stage_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shader_stages[]      = { vert_stage_info, frag_stage_info };
    VkPipelineVertexInputStateCreateInfo vert_input_info = {0};
    vert_input_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vert_input_info.vertexBindingDescriptionCount        = desc->vertex_layout.binding_count;
    vert_input_info.pVertexBindingDescriptions           = bind_desc;
    vert_input_info.vertexAttributeDescriptionCount      = attr_count;
    vert_input_info.pVertexAttributeDescriptions         = attr_desc;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ARRAY_LEN(dynamic_states),
        .pDynamicStates = dynamic_states,
    };
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,   // pointers stay NULL, values come at record time
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0F;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0F;
    rasterizer.depthBiasClamp          = 0.0F;
    rasterizer.depthBiasSlopeFactor    = 0.0F;

    VkPipelineMultisampleStateCreateInfo multisampling;
    memset(&multisampling, 0, sizeof(multisampling));
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0F;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;
    multisampling.pNext                 = nullptr;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask      =
        VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend = {0};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_COPY;
    color_blend.attachmentCount = 1;
    color_blend.pAttachments = &color_blend_attachment;
    color_blend.blendConstants[0] = 0.0F;
    color_blend.blendConstants[1] = 0.0F;
    color_blend.blendConstants[2] = 0.0F;
    color_blend.blendConstants[3] = 0.0F;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    VkPipelineLayout pipeline_layout;
    pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount             = 0;
    pipeline_layout_info.pSetLayouts                = nullptr;
    pipeline_layout_info.pushConstantRangeCount     = 0;
    pipeline_layout_info.pPushConstantRanges        = nullptr;
    VK_TRY(vkCreatePipelineLayout(dev->ctx.logical_device, &pipeline_layout_info, nullptr, &pipeline_layout));
    dev->graphics_pipeline.layout = pipeline_layout;

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount                   = 2;
    pipeline_info.pStages                      = shader_stages;
    pipeline_info.pVertexInputState            = &vert_input_info;
    pipeline_info.pInputAssemblyState          = &input_assembly;
    pipeline_info.pViewportState               = &viewport_state;
    pipeline_info.pRasterizationState          = &rasterizer;
    pipeline_info.pMultisampleState            = &multisampling;
    pipeline_info.pDepthStencilState           = nullptr;
    pipeline_info.pColorBlendState             = &color_blend;
    pipeline_info.pDynamicState                = &dynamic;
    pipeline_info.layout                       = pipeline_layout;
    pipeline_info.renderPass                   = dev->render_pass;
    pipeline_info.subpass                      = 0;
    pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex            = -1;
    VK_TRY(vkCreateGraphicsPipelines(dev->ctx.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &dev->graphics_pipeline.handle));
    vkDestroyShaderModule(dev->ctx.logical_device, vert_shader, nullptr);
    vkDestroyShaderModule(dev->ctx.logical_device, frag_shader, nullptr);
    return RESULT_OK;
}

Result
vulkan_swapchain_init(GfxDevice_T *dev, GfxWindow *window) {
    VkSwapchainCreateInfoKHR swapchain_info;
    int32_t width = 0;
    int32_t height = 0;
    Swapchain *swapchain    = &dev->swapchain;
    VkContext *ctx          = &dev->ctx;

    vulkan_platform_framebuffer_size_get(window, &width, &height);
    TRY(vulkan_swapchain_info_create(
        dev->ctx.physical_device,
        dev->ctx.surface,
        width,
        height,
        &swapchain_info,
        &dev->swapchain.image_format,
        &dev->swapchain.extent
    ));
    VK_TRY(vkCreateSwapchainKHR(ctx->logical_device, &swapchain_info, nullptr, &swapchain->handle));

    uint32_t image_count = 0;
    VK_TRY(vkGetSwapchainImagesKHR(ctx->logical_device, swapchain->handle, &image_count, nullptr));
    TRY(tda_create(&swapchain->images, image_count));
    VK_TRY(vkGetSwapchainImagesKHR(
        ctx->logical_device,
        swapchain->handle,
        &image_count,
        tda_data(&swapchain->images)
    ));
    tda_size(&swapchain->images) = image_count;

    TRY(vulkan_swapchain_image_views_create_from_image(ctx->logical_device, swapchain->image_format, &swapchain->images, &swapchain->image_views));

    return RESULT_OK;
}

Result
vulkan_context_init(GfxDevice_T *dev, VkContextCreateInfo *ctx_info) {
    VkContext *ctx = &dev->ctx;
    VkInstanceCreateInfo create_info    = {0};
    VkApplicationInfo dev_info          = {0};

    dev_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    dev_info.pApplicationName   = "Hello Triangle";
    dev_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    dev_info.pEngineName        = "No Engine";
    dev_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    dev_info.apiVersion         = VK_API_VERSION_1_3;

    create_info.sType               = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo    = &dev_info;
    create_info.enabledLayerCount   = ctx_info->layers_count;
    create_info.ppEnabledLayerNames = ctx_info->validation_layers;
    TRY(vulkan_validation_layers_check(ctx_info->validation_layers, ctx_info->layers_count));

    DynamicArray da = {0};
    uint32_t    extensions_count;
    const char **extensions = vulkan_platform_instance_extensions_get(&extensions_count);
    TRY(da_create(&da, sizeof(*extensions), 16));

    for (uint32_t i = 0; i < extensions_count; ++i) {
        TRY(da_push(&da, (void *)&extensions[i]));
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_mes_info = {0};

    if (debug_mode) {
        const char *dbg_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        TRY(da_push(&da, (void *)&dbg_ext));
        vulkan_debug_messenger_populate(&debug_mes_info);
        create_info.pNext = &debug_mes_info;
    }

    create_info.enabledExtensionCount   = da.size;
    create_info.ppEnabledExtensionNames = (const char **)da.data;
    create_info.enabledLayerCount       = 0;
    create_info.enabledLayerCount       = ctx_info->layers_count;
    create_info.ppEnabledLayerNames     = ctx_info->validation_layers;

    VK_TRY(vkCreateInstance(&create_info, nullptr, &ctx->instance));

    if (debug_mode) {
        TRY(vulkan_debug_messenger_create(ctx->instance, &debug_mes_info, &ctx->messenger));
    }

    TRY(vulkan_platform_surface_create(ctx->instance, ctx_info->window, &ctx->surface));
    TRY(vulkan_device_pick(ctx->instance, ctx->surface, &ctx->physical_device));
    TRY(vulkan_logical_device_create(ctx->physical_device, ctx->surface, &ctx->logical_device));

    QueueFamilyIndices indices = vulkan_device_find_queue_families(ctx->physical_device, ctx->surface);
    vkGetDeviceQueue(ctx->logical_device, indices.graphics_family, 0, &ctx->graphics_queue);
    vkGetDeviceQueue(ctx->logical_device, indices.present_family, 0, &ctx->present_queue);

    da_destroy(&da);
    return RESULT_OK;
}

Result
vulkan_frame_init(VkContext *ctx, FrameData *frame) {
    Array(VkSemaphorePtr) semaphores = {0};
    tda_from(&semaphores, (void*)((VkSemaphore*[]){&frame->image_available_semaphore, &frame->render_finished_semaphore}), 2);
    Array(VkFencePtr) fences = {0};
    tda_from(&fences, (void *)(VkFence*[]){&frame->in_flight_fence}, 1);
    TRY(vulkan_sync_objects_create(ctx->logical_device, &semaphores, &fences));

    return RESULT_OK;
}

void
gfx_device_destroy(GfxDevice_T *dev) {
    VkContext *ctx = &dev->ctx;
    vkDeviceWaitIdle(ctx->logical_device);
    vulkan_debug_messenger_destroy(ctx->instance, ctx->messenger);
    // vkDestroySemaphore(dev->ctx.logical_device, dev->image_available_semaphore, nullptr);
    // vkDestroySemaphore(dev->ctx.logical_device, dev->render_finished_semaphore, nullptr);
    // vkDestroyFence(dev->ctx.logical_device, dev->in_flight_fence, nullptr);
    vkDestroyCommandPool(ctx->logical_device, dev->command_pool, nullptr);
    // for (size_T i = 0; i < tda_size(&dev->buffers); i++) {
    //     vkDestroyBuffer(dev->ctx.logical_device, *tda_at(&dev->buffers, i), nullptr);
    // }
    for (size_t i = 0; i < tda_size(&dev->device_memory); i++) {
        vkFreeMemory(ctx->logical_device, *tda_at(&dev->device_memory, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&dev->swapchain.framebuffers); i++) {
        vkDestroyFramebuffer(ctx->logical_device, *tda_at(&dev->swapchain.framebuffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&dev->swapchain.image_views); i++) {
        vkDestroyImageView(ctx->logical_device, *tda_at(&dev->swapchain.image_views, i), nullptr);
    }
    // tda_destroy(&dev->buffers);
    tda_destroy(&dev->swapchain.framebuffers)
    tda_destroy(&dev->swapchain.image_views);
    tda_destroy(&dev->swapchain.images);
    vkDestroyPipeline(ctx->logical_device, dev->graphics_pipeline.handle, nullptr);
    vkDestroyPipelineLayout(ctx->logical_device, dev->graphics_pipeline.layout, nullptr);
    vkDestroyRenderPass(ctx->logical_device, dev->render_pass, nullptr);
    vkDestroySwapchainKHR(ctx->logical_device, dev->swapchain.handle, nullptr);
    vkDestroyDevice(ctx->logical_device, nullptr);
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, nullptr);
    vkDestroyInstance(ctx->instance, nullptr);
}

Result
gfx_device_create(GfxDeviceDesc *dev_info, GfxDevice_T **device) {
    Result r = RESULT_OK;
    assert(device != nullptr);
    assert(*device == nullptr);
    assert(dev_info != nullptr);

    GfxDevice_T *dev = alloc(sizeof(*dev));
    *device = dev;

    VkContextCreateInfo ctx_info = {
        .window = dev_info->window,
    };

    TRY_GOTO(r, fail, vulkan_context_init(dev, &ctx_info));
    TRY_GOTO(r, fail, vulkan_swapchain_init(dev, ctx_info.window));
    TRY_GOTO(r, fail, vulkan_render_pass_create(dev->ctx.logical_device, &dev->swapchain.image_format, &dev->render_pass));
    TRY_GOTO(r, fail, vulkan_framebuffers_create(dev->ctx.logical_device, dev->render_pass, dev->swapchain.extent, &dev->swapchain.image_views, &dev->swapchain.framebuffers));
    QueueFamilyIndices indices = vulkan_device_find_queue_families(dev->ctx.physical_device, dev->ctx.surface);
    TRY(vulkan_command_pool_create(dev->ctx.logical_device, indices.graphics_family, &dev->command_pool));
    return RESULT_OK;

fail:
    gfx_device_destroy(dev);
    return r;
}

struct GfxBuffer_T {
    VkBuffer        handle;
    size_t          size;
    GfxBufferUsage  usage;
};

Result
gfx_device_memory_request(GfxDevice dev) {

}

Result gfx_buffer_create(GfxDevice dev, GfxBufferDesc *desc, GfxBuffer *out) {
    Result r = RESULT_OK;

    GfxBuffer_T *gfx_buffer = alloc(sizeof(*gfx_buffer));
    gfx_buffer->size = desc->size;
    gfx_buffer->usage = desc->usage;

    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = desc->size;
    buffer_info.usage = desc->usage;

    VkBuffer buf = {0};

    TRY_GOTO(r, fail, vulkan_buffer_create(dev->ctx.logical_device, &buffer_info, &buf));
    VkMemoryRequirements mem_req = vulkan_buffer_memory_requirements_get(dev->ctx.logical_device, buf);

    TRY_GOTO(r, fail, vulkan_memory_allocate(dev->ctx.logical_device, dev->ctx.physical_device, mem_req, device_memory));

    *out = gfx_buffer;
    return RESULT_OK;

fail:
    free(gfx_buffer);
    return r;
}
