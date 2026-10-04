#include <utils/result_tools.h>
#include <utils/utils.h>
#include <interface/mage_gfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "interface/mage_native_window.h"
#include "gfx_vulkan_internal.h"
#include "interface/mage_result.h"
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

    uint32_t ws_mask          = {0};
    Array(CharPtr) extensions = {0};
    char **extout             = {0};
    uint32_t count            = {0};
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

    VK_TRY(vkCreateInstance(&create_info, NULL, &ctx->instance));

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
    gfx_swapchain_destroy(dev);

    for (size_t i = 0; i < ARRAY_LEN(dev->frames); i++) {
        vkDestroySemaphore(dev->ctx.logical_device, dev->frames[i].image_available, NULL);
        vkDestroyFence(dev->ctx.logical_device, dev->frames[i].in_flight_fence, NULL);
        vkDestroyCommandPool(ctx->logical_device, dev->frames[i].command_pool, NULL);
    }
    for (size_t i = 0; i < ARRAY_LEN(dev->global_buffers); i++) {
        gfx_buffer_destroy(dev, dev->global_buffers[i]);
    }
    vkDestroyDescriptorPool(dev->ctx.logical_device, dev->descriptors.pool, NULL);
    vkDestroyRenderPass(ctx->logical_device, dev->render_pass, NULL);
    gfx_memory_pool_destroy(dev, &dev->pool_gpu);
    gfx_memory_pool_destroy(dev, &dev->pool_upload);
    vkDestroyDescriptorSetLayout(dev->ctx.logical_device, dev->descriptors.layout, NULL);
    vkDestroyDevice(ctx->logical_device, NULL);
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);

    vulkan_debug_messenger_destroy(ctx->instance, ctx->messenger);
    vkDestroyInstance(ctx->instance, NULL);
    free(dev);
}

Result
gfx_device_create(GfxDeviceDesc *dev_info, GfxDevice_T **device) {
    Result r = RESULT_OK;
    assert(device != NULL);
    assert(*device == NULL);
    assert(dev_info != NULL);

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
    TRY_GOTO(r, fail, gfx_vulkan_descriptor_layout_init(dev));
    TRY_GOTO(r, fail, vulkan_render_pass_create(dev->ctx.logical_device, &dev->ctx.surface_format.format, &dev->render_pass));
    TRY_GOTO(r, fail, gfx_swapchain_create(dev, dev->width, dev->height));
    TRY_GOTO(r, fail, gfx_memory_pool_create(
        dev,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        GPU_POOL_GPU_BLOCK_SIZE,
        GPU_BLOCK_COUNT,
        &dev->pool_gpu
    ));
    TRY_GOTO(r, fail, gfx_memory_pool_create(
        dev,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        GPU_POOL_GPU_BLOCK_SIZE,
        GPU_BLOCK_COUNT,
        &dev->pool_upload
    ));
    TRY_GOTO(r, fail, gfx_vulkan_descriptor_pool_create(dev));
    TRY_GOTO(r, fail, gfx_vulkan_descriptor_set_create(dev));

    for (size_t i = 0; i < GFX_MAX_FRAMES; i++) {
        TRY(gfx_framedata_init(&dev->ctx, &dev->frames[i]));
    }
    return RESULT_OK;

fail:
    gfx_device_destroy(dev);
    free(dev);
    return r;
}
