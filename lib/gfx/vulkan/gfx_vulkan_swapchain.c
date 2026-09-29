#include "gfx_vulkan_internal.h"
#include <utils/result_tools.h>
#include <string.h>

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

Result
gfx_swapchain_init(GfxDevice_T *dev, uint32_t width, uint32_t height) {
    VkSwapchainCreateInfoKHR swapchain_info;
    memset(&swapchain_info, 0, sizeof(swapchain_info));

    Swapchain *swapchain    = &dev->swapchain;
    VkContext *ctx          = &dev->ctx;

    TRY(vulkan_swapchain_info_create(
        dev->ctx.physical_device,
        dev->ctx.surface,
        width,
        height,
        &dev->ctx.indices,
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
    TRY(vulkan_framebuffers_create(ctx->logical_device, dev->render_pass, swapchain->extent, &swapchain->image_views, &swapchain->framebuffers));
    TRY(gfx_swapchain_semaphores_create(dev));

    return RESULT_OK;
}

void
gfx_swapchain_destroy(GfxDevice_T *dev) {
    VkDevice device = dev->ctx.logical_device;
    Swapchain *swapchain = &dev->swapchain;

    for (size_t i = 0; i < tda_size(&swapchain->framebuffers); i++) {
        vkDestroyFramebuffer(device, *tda_at(&swapchain->framebuffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&swapchain->image_views); i++) {
        vkDestroyImageView(device, *tda_at(&swapchain->image_views, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&swapchain->render_finished); i++) {
        vkDestroySemaphore(device, *tda_at(&swapchain->render_finished, i), nullptr);
    }


    tda_destroy(&swapchain->framebuffers);
    tda_destroy(&swapchain->images);
    tda_destroy(&swapchain->image_views);
    tda_destroy(&swapchain->render_finished);

    if (swapchain->handle) {
        vkDestroySwapchainKHR(device, swapchain->handle, nullptr);
    }
    swapchain->handle = VK_NULL_HANDLE;
}

Result
gfx_swapchain_recreate(GfxDevice dev) {
    VkContext *ctx = &dev->ctx;
    Swapchain *swapchain = &dev->swapchain;
    if (dev->height == 0 || dev->width == 0) {
        dev->swapchain_dirty = true;
        return RESULT_OK;
    }

    VkDevice device = dev->ctx.logical_device;

    VK_TRY(vkDeviceWaitIdle(device));
    gfx_swapchain_destroy(dev);

    TRY(gfx_swapchain_init(dev, dev->width, dev->height));
    TRY(vulkan_swapchain_image_views_create_from_image(ctx->logical_device, swapchain->image_format, &swapchain->images, &swapchain->image_views));
    TRY(vulkan_framebuffers_create(ctx->logical_device, dev->render_pass, swapchain->extent, &swapchain->image_views, &swapchain->framebuffers));
    dev->swapchain_dirty = false;
    return RESULT_OK;
}

