#include <utils/result_tools.h>
#include <utils/utils.h>
#include <interface/gfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "interface/native_window.h"
#include "gfx_vulkan_internal.h"
#include "core/result.h"
#include "vk/vk.h"
#include <utils/da.h>

const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
};

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

Result
gfx_vulkan_context_init(GfxDevice_T *dev, VkContextCreateInfo *ctx_info) {
    VkContext *ctx = &dev->ctx;
    VkInstanceCreateInfo create_info    = {0};
    VkApplicationInfo dev_info          = {0};
    const CoreNativeWindow *window      = ctx_info->window;

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

    uint32_t ws_mask = {0};
    Array(CharPtr) extensions = {0};
    char **extout = {0};
    uint32_t count = {0};
    if (window) {
        gfx_platform_surface_instance_extensions_get(&ws_mask, &extout, &count);
    }
    tda_from(&extensions, (void *)extout, count);

    VkDebugUtilsMessengerCreateInfoEXT debug_mes_info = {0};
    if (dev->debug_mode) {
        const char *dbg_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        TRY(tda_push(&extensions, (void *)&dbg_ext));
        vulkan_debug_messenger_populate(&debug_mes_info);
        create_info.pNext = &debug_mes_info;
    }

    create_info.enabledExtensionCount   = tda_size(&extensions);
    create_info.ppEnabledExtensionNames = (const char **)tda_data(&extensions);
    create_info.enabledLayerCount       = 0;
    create_info.enabledLayerCount       = ctx_info->layers_count;
    create_info.ppEnabledLayerNames     = ctx_info->validation_layers;

    VK_TRY(vkCreateInstance(&create_info, nullptr, &ctx->instance));

    if (dev->debug_mode) {
        TRY(vulkan_debug_messenger_create(ctx->instance, &debug_mes_info, &ctx->messenger));
    }

    const char *device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    };

    Array(CharPtr) ext;
    tda_from(&ext, (void *)device_extensions, ARRAY_LEN(device_extensions));


    TRY(gfx_platform_surface_create(dev, ws_mask, window));
    TRY(vulkan_device_pick(ctx->instance, ctx->surface, &ext, &ctx->physical_device));
    TRY(vulkan_logical_device_create(ctx->physical_device, ctx->surface, &ext, &ctx->logical_device));

    TRY(vulkan_swapchain_image_format_query(dev->ctx.physical_device, dev->ctx.surface, &dev->ctx.surface_format));

    ctx->indices = vulkan_device_find_queue_families(ctx->physical_device, ctx->surface);
    vkGetDeviceQueue(ctx->logical_device, ctx->indices.graphics_family, 0, &ctx->graphics_queue);
    vkGetDeviceQueue(ctx->logical_device, ctx->indices.present_family, 0, &ctx->present_queue);

    tda_destroy(&extensions);
    return RESULT_OK;
}

Result
gfx_framedata_init(VkContext *ctx, FrameData *frame) {
    TRY(vulkan_semaphore_create(ctx->logical_device, &frame->image_available));
    TRY(vulkan_fence_create(ctx->logical_device, &frame->in_flight_fence));
    TRY(vulkan_command_pool_create(ctx->logical_device, ctx->indices.graphics_family, &frame->command_pool));
    TRY(vulkan_command_buffer_create(ctx->logical_device, frame->command_pool, &frame->command_buffer));
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
    // for (size_t i = 0; i < tda_size(&dev->device_memory); i++) {
    //     vkFreeMemory(ctx->logical_device, *tda_at(&dev->device_memory, i), nullptr);
    // }
    for (size_t i = 0; i < tda_size(&dev->swapchain.framebuffers); i++) {
        vkDestroyFramebuffer(ctx->logical_device, *tda_at(&dev->swapchain.framebuffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&dev->swapchain.image_views); i++) {
        vkDestroyImageView(ctx->logical_device, *tda_at(&dev->swapchain.image_views, i), nullptr);
    }
    // tda_destroy(&dev->buffers);
    tda_destroy(&dev->swapchain.framebuffers);
    tda_destroy(&dev->swapchain.image_views);
    tda_destroy(&dev->swapchain.images);
    vkDestroyRenderPass(ctx->logical_device, dev->render_pass, nullptr);
    vkDestroySwapchainKHR(ctx->logical_device, dev->swapchain.handle, nullptr);
    vkDestroyDevice(ctx->logical_device, nullptr);
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, nullptr);
    vkDestroyInstance(ctx->instance, nullptr);
}

void
gfx_pipeline_destroy(GfxDevice dev, GfxPipeline pipeline) {
    vkDestroyPipeline(dev->ctx.logical_device, pipeline->handle, nullptr);
    vkDestroyPipelineLayout(dev->ctx.logical_device, pipeline->layout, nullptr);
}

Result
gfx_device_create(GfxDeviceDesc *dev_info, GfxDevice_T **device) {
    Result r = RESULT_OK;
    assert(device != nullptr);
    assert(*device == nullptr);
    assert(dev_info != nullptr);

    GfxDevice_T *dev = alloc(sizeof(*dev));
    dev->debug_mode = dev_info->debug_mode;
    dev->width = dev_info->width;
    dev->height = dev_info->height;

    *device = dev;

    const char *validation_layers[1] = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkContextCreateInfo ctx_info = {
        .layers_count = dev_info->debug_mode ? ARRAY_LEN(validation_layers) : 0,
        .validation_layers = dev_info->debug_mode ? validation_layers : 0,
        .window = dev_info->window,
    };

    TRY_GOTO(r, fail, gfx_vulkan_context_init(dev, &ctx_info));
    TRY_GOTO(r, fail, vulkan_render_pass_create(dev->ctx.logical_device, &dev->ctx.surface_format.format, &dev->render_pass));
    TRY_GOTO(r, fail, gfx_swapchain_init(dev, dev->width, dev->height));
    TRY_GOTO(r, fail, vulkan_framebuffers_create(
        dev->ctx.logical_device,
        dev->render_pass,
        dev->swapchain.extent,
        &dev->swapchain.image_views,
        &dev->swapchain.framebuffers
    ));
    TRY_GOTO(r, fail, gfx_memory_pool_init(
        dev,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        GPU_POOL_GPU_BLOCK_SIZE,
        GPU_BLOCK_COUNT,
        &dev->pool_gpu
    ));
    TRY_GOTO(r, fail, gfx_memory_pool_init(
        dev,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        GPU_POOL_GPU_BLOCK_SIZE,
        GPU_BLOCK_COUNT,
        &dev->pool_upload
    ));

    for (size_t i = 0; i < GFX_FRAME_COUNT; i++) {
        TRY(gfx_framedata_init(&dev->ctx, &dev->frames[i]));
    }

    TRY(vulkan_command_pool_create(dev->ctx.logical_device, dev->ctx.indices.graphics_family, &dev->command_pool));
    return RESULT_OK;

fail:
    gfx_device_destroy(dev);
    free(dev);
    return r;
}
