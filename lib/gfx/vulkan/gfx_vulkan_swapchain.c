#include "gfx_vulkan_internal.h"
#include <utils/result_tools.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static Result
gfx_swapchain_semaphores_create(GfxDevice_T *dev) {
    Swapchain *swapchain = &dev->swapchain;

    uint32_t image_count = tda_size(&swapchain->images);
    tda_create(&swapchain->render_finished, image_count);
    for (uint32_t i = 0; i < image_count; i++) {
        VkSemaphore semaphore = {0};
        TRY(vulkan_semaphore_create(dev->ctx.logical_device, &semaphore));
        tda_push(&swapchain->render_finished, (void *)&semaphore);
    }

    return RESULT_OK;
}

static Result
gfx_depth_create(GfxDevice_T *dev) {
    VkDevice  device = dev->ctx.logical_device;
    Swapchain *sc    = &dev->swapchain;
    Result    r      = RESULT_OK;

    VkImageCreateInfo ii = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .extent = { sc->extent.width, sc->extent.height, 1 },
        .mipLevels = 1, .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VK_TRY_GOTO(r, fail, vkCreateImage(device, &ii, NULL, &sc->depth_image));

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(device, sc->depth_image, &req);
    uint32_t mem_type;
    TRY_GOTO(r, fail, vulkan_memory_type_find(dev->ctx.physical_device, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_type));
    VkMemoryAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = req.size,
        .memoryTypeIndex = mem_type,
    };
    VK_TRY_GOTO(r, fail, vkAllocateMemory(device, &ai, NULL, &sc->depth_memory));
    VK_TRY_GOTO(r, fail, vkBindImageMemory(device, sc->depth_image, sc->depth_memory, 0));

    VkImageViewCreateInfo vi = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = sc->depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 },
    };
    VK_TRY_GOTO(r, fail, vkCreateImageView(device, &vi, NULL, &sc->depth_view));
    return r;
fail:
    return r;
}

Result
gfx_swapchain_create(GfxDevice_T *dev, uint32_t width, uint32_t height) {
    Result r = RESULT_OK;

    VkSwapchainCreateInfoKHR swapchain_info;
    memset(&swapchain_info, 0, sizeof(swapchain_info));

    Swapchain *swapchain    = &dev->swapchain;
    VkContext *ctx          = &dev->ctx;

    TRY_GOTO(r, fail, vulkan_swapchain_info_create(
        dev->ctx.physical_device,
        dev->ctx.surface,
        width,
        height,
        &dev->ctx.indices,
        &swapchain_info,
        &dev->swapchain.image_format,
        &dev->swapchain.extent
    ));
    if (swapchain) {
        swapchain_info.oldSwapchain = swapchain->handle;
    }
    VK_TRY_GOTO(r, fail, vkCreateSwapchainKHR(ctx->logical_device, &swapchain_info, NULL, &swapchain->handle));

    uint32_t image_count = 0;
    VK_TRY_GOTO(r, fail, vkGetSwapchainImagesKHR(ctx->logical_device, swapchain->handle, &image_count, NULL));
    TRY_GOTO(r, fail, tda_create(&swapchain->images, image_count));
    VK_TRY_GOTO(r, fail, vkGetSwapchainImagesKHR(
        ctx->logical_device,
        swapchain->handle,
        &image_count,
        tda_data(&swapchain->images)
    ));
    tda_size(&swapchain->images) = image_count;

    TRY_GOTO(r, fail, vulkan_swapchain_image_views_create_from_image(ctx->logical_device, swapchain->image_format, &swapchain->images, &swapchain->image_views));
    TRY_GOTO(r, fail, gfx_depth_create(dev));
    TRY_GOTO(r, fail, vulkan_framebuffers_create(ctx->logical_device, dev->render_pass, swapchain->extent, swapchain->depth_view, &swapchain->image_views, &swapchain->framebuffers));
    TRY_GOTO(r, fail, gfx_swapchain_semaphores_create(dev));

    return RESULT_OK;
fail:
    gfx_swapchain_destroy(dev);
    return r;
}

static void
gfx_swapchain_resources_destroy(GfxDevice_T *dev) {
    VkDevice   device = dev->ctx.logical_device;
    Swapchain *sc     = &dev->swapchain;

    for (size_t i = 0; i < tda_size(&sc->framebuffers); i++) {
        vkDestroyFramebuffer(device, *tda_at(&sc->framebuffers, i), NULL);
    }
    for (size_t i = 0; i < tda_size(&sc->image_views); i++) {
        vkDestroyImageView(device, *tda_at(&sc->image_views, i), NULL);
    }
    vkDestroyImageView(device, sc->depth_view, NULL);
    vkDestroyImage(device, sc->depth_image, NULL);
    vkFreeMemory(device, sc->depth_memory, NULL);
    for (size_t i = 0; i < tda_size(&sc->render_finished); i++) {
        vkDestroySemaphore(device, *tda_at(&sc->render_finished, i), NULL);
    }

    tda_destroy(&sc->framebuffers);
    tda_destroy(&sc->images);
    tda_destroy(&sc->image_views);
    tda_destroy(&sc->render_finished);

    VkSwapchainKHR handle = sc->handle;
    *sc = (Swapchain){0};
    sc->handle = handle;
}

void
gfx_swapchain_destroy(GfxDevice_T *dev) {
    vkDeviceWaitIdle(dev->ctx.logical_device);
    gfx_swapchain_resources_destroy(dev);
    vkDestroySwapchainKHR(dev->ctx.logical_device, dev->swapchain.handle, NULL);
    dev->swapchain.handle = VK_NULL_HANDLE;
}

Result
gfx_swapchain_recreate(GfxDevice dev) {
    Result r = RESULT_OK;
    if (dev->height == 0 || dev->width == 0) {
        dev->swapchain_dirty = true;
        return RESULT_OK;
    }
    VK_TRY_GOTO(r, fail, vkDeviceWaitIdle(dev->ctx.logical_device));

    gfx_swapchain_resources_destroy(dev);
    VkSwapchainKHR old = dev->swapchain.handle;

    r = gfx_swapchain_create(dev, dev->width, dev->height);
    vkDestroySwapchainKHR(dev->ctx.logical_device, old, NULL);
    if (r != RESULT_OK) goto fail;

    dev->swapchain_dirty = false;
    return r;
fail:
    gfx_swapchain_destroy(dev);
    return r;
}
