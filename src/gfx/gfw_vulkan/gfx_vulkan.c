#include <gfx/gfx.h>
#include <stdint.h>
#include "vk.h"

struct GfxDevice {
    VkContext                   ctx;
    Array(VkDeviceMemory)       device_memory;
    GraphicsPipeline            graphics_pipeline;
    Swapchain                   swapchain;
    FrameData                   frames;
    VkCommandPool               command_pool;
    VkRenderPass                render_pass;
};

typedef struct {
    const char **validation_layers;
    uint32_t layers_count;
    GfxWindow *window;
} VkContextCreateInfo;

Result
vulkan_context_init(VkContext *ctx, VkContextCreateInfo *ctx_info) {
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
vulkan_swapchain_init(VkContext *ctx, Swapchain *swapchain, VkContextCreateInfo *ctx_info) {
    VkSwapchainCreateInfoKHR swapchain_info;
    int32_t width = 0;
    int32_t height = 0;

    vulkan_platform_framebuffer_size_get(ctx_info->window, &width, &height);
    TRY(vulkan_swapchain_info_create(
        ctx->physical_device,
        ctx->surface,
        width,
        height,
        &swapchain_info,
        &swapchain->image_format,
        &swapchain->extent
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
vulkan_frame_init(VkContext *ctx, FrameData *frame) {
    Array(VkSemaphorePtr) semaphores = {0};
    TRY(tda_from(&semaphores, (void*)((VkSemaphore*[]){&frame->image_available_semaphore, &frame->render_finished_semaphore}), 2));
    Array(VkFencePtr) fences = {0};
    TRY(tda_from(&fences, (void *)(VkFence*[]){&frame->in_flight_fence}, 1));
    TRY(vulkan_sync_objects_create(ctx->logical_device, &semaphores, &fences));

    return RESULT_OK;
}

void
gfx_device_destroy(GfxDevice *dev) {
    vkDeviceWaitIdle(dev->ctx.logical_device);
    vulkan_debug_messenger_destroy(dev->ctx.instance, dev->ctx.messenger);
    // vkDestroySemaphore(dev->ctx.logical_device, dev->image_available_semaphore, nullptr);
    // vkDestroySemaphore(dev->ctx.logical_device, dev->render_finished_semaphore, nullptr);
    // vkDestroyFence(dev->ctx.logical_device, dev->in_flight_fence, nullptr);
    vkDestroyCommandPool(dev->ctx.logical_device, dev->command_pool, nullptr);
    // for (size_t i = 0; i < tda_size(&dev->buffers); i++) {
    //     vkDestroyBuffer(dev->ctx.logical_device, *tda_at(&dev->buffers, i), nullptr);
    // }
    for (size_t i = 0; i < tda_size(&dev->device_memory); i++) {
        vkFreeMemory(dev->ctx.logical_device, *tda_at(&dev->device_memory, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&dev->swapchain.framebuffers); i++) {
        vkDestroyFramebuffer(dev->ctx.logical_device, *tda_at(&dev->swapchain.framebuffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&dev->swapchain.image_views); i++) {
        vkDestroyImageView(dev->ctx.logical_device, *tda_at(&dev->swapchain.image_views, i), nullptr);
    }
    // tda_destroy(&dev->buffers);
    tda_destroy(&dev->swapchain.framebuffers)
    tda_destroy(&dev->swapchain.image_views);
    tda_destroy(&dev->swapchain.images);
    vkDestroyPipeline(dev->ctx.logical_device, dev->graphics_pipeline.handle, nullptr);
    vkDestroyPipelineLayout(dev->ctx.logical_device, dev->graphics_pipeline.layout, nullptr);
    vkDestroyRenderPass(dev->ctx.logical_device, dev->graphics_pipeline.render_pass, nullptr);
    vkDestroySwapchainKHR(dev->ctx.logical_device, dev->swapchain.handle, nullptr);
    vkDestroyDevice(dev->ctx.logical_device, nullptr);
    vkDestroySurfaceKHR(dev->ctx.instance, dev->ctx.surface, nullptr);
    vkDestroyInstance(dev->ctx.instance, nullptr);
}

Result
gfx_device_create(GfxDevice *dev, GfxWindow *window, const char **validation_layers, uint32_t layers_count) {
    VkContextCreateInfo ctx_info = {
        .validation_layers = validation_layers,
        .layers_count = layers_count,
        .window = window,
    };
    TRY(vulkan_context_init(&dev->ctx, &ctx_info));
    TRY(vulkan_render_pass_create(dev->ctx.logical_device, &dev->swapchain.image_format, &dev->render_pass));
    TRY(vulkan_framebuffers_create(dev->ctx.logical_device, dev->render_pass, dev->swapchain.extent, &dev->swapchain.image_views, &dev->swapchain.framebuffers));
    TRY(vulkan_graphics_pipeline_create(dev->ctx.logical_device, dev->graphics_pipeline.render_pass, dev->swapchain.extent, &dev->graphics_pipeline.layout, &dev->graphics_pipeline.handle));
    QueueFamilyIndices indices = vulkan_device_find_queue_families(dev->ctx.physical_device, dev->ctx.surface);
    TRY(vulkan_command_pool_create(dev->ctx.logical_device, indices.graphics_family, &dev->command_pool));

    return RESULT_OK;
}
