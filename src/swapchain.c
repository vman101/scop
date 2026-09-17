#include "scop.h"
#include <limits.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkSurfaceFormatKHR
vulkan_swapchain_surface_format_choose(SwapChainSupportDetails *details) {
    assert(details->formats.size > 0);
    for (size_t i = 0; i < details->formats.size; i++) {
        VkSurfaceFormatKHR *surfaceFormat = da_get(&details->formats, i);
        if (surfaceFormat->format == VK_FORMAT_B8G8R8A8_SRGB
            && surfaceFormat->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ) {
            return *surfaceFormat;
        }
    }

    return *(VkSurfaceFormatKHR *)da_get(&details->formats, 0);
}

VkPresentModeKHR
vulkan_swapchain_present_mode_choose(SwapChainSupportDetails *details) {
    for (size_t i = 0; i < details->present_modes.size; i++) {
        VkPresentModeKHR *presentMode = (VkPresentModeKHR *)da_get(&details->present_modes, i);
        if (*presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return *presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

int32_t clamp(int32_t n, int32_t min, int32_t max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

uint32_t uclamp(uint32_t n, uint32_t min, uint32_t max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

VkExtent2D vulkan_swapchain_extend_choose(SwapChainSupportDetails *details, uint32_t width, uint32_t height) {
    if (details->capabilities.currentExtent.width != UINT_MAX) {
        return details->capabilities.currentExtent;
    }
    VkExtent2D actual_extend = {
        .width = width,
        .height = height,
    };
    actual_extend.width = uclamp(
        actual_extend.width,
        details->capabilities.minImageExtent.width,
        details->capabilities.maxImageExtent.width
    );
    actual_extend.height = uclamp(
        actual_extend.height,
        details->capabilities.minImageExtent.height,
        details->capabilities.maxImageExtent.height
    );
    return actual_extend;
}

Result
vulkan_swapchain_support_query(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails *details) {
    VK_TRY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities));

    uint32_t format_count = 0;
    VK_TRY(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr));
    TRY(da_create(&details->formats, sizeof(VkSurfaceFormatKHR), format_count));
    VK_TRY(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details->formats.data));
    details->formats.size = format_count;

    uint32_t present_mode_count = 0;
    VK_TRY(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr));
    TRY(da_create(&details->present_modes, sizeof(VkPresentModeKHR), present_mode_count));
    VK_TRY(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details->present_modes.data));
    details->present_modes.size = present_mode_count;

    return RESULT_OK;
}

Result
vulkan_swapchain_info_create(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    uint32_t width,
    uint32_t height,
    VkSwapchainCreateInfoKHR *swapchain_info,
    VkFormat *swapchain_format,
    VkExtent2D *swapchain_extent
) {
    SwapChainSupportDetails details;
    TRY(vulkan_swapchain_support_query(device, surface, &details));

    VkSurfaceFormatKHR format = vulkan_swapchain_surface_format_choose(&details);
    VkPresentModeKHR present_mode = vulkan_swapchain_present_mode_choose(&details);
    VkExtent2D extent = vulkan_swapchain_extend_choose(&details, width, height);

    uint32_t image_count = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }

    swapchain_info->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info->surface = surface;
    swapchain_info->minImageCount = image_count;
    swapchain_info->imageFormat = format.format;
    swapchain_info->imageColorSpace = format.colorSpace;
    swapchain_info->imageExtent = extent;
    swapchain_info->imageArrayLayers = 1;
    swapchain_info->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = vulkan_device_find_queue_families(device, surface);
    uint32_t queue_family_indices[] = { indices.graphics_family, indices.present_family };

    if (indices.graphics_family != indices.present_family) {
        swapchain_info->imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info->queueFamilyIndexCount = 2;
        swapchain_info->pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_info->imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info->queueFamilyIndexCount = 0;
        swapchain_info->pQueueFamilyIndices = nullptr;
    }

    swapchain_info->preTransform = details.capabilities.currentTransform;
    swapchain_info->compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info->presentMode = present_mode;
    swapchain_info->clipped = VK_TRUE;
    swapchain_info->oldSwapchain = VK_NULL_HANDLE;

    *swapchain_format = format.format;
    *swapchain_extent = extent;

    return RESULT_OK;
}

Result
vulkan_swapchain_image_view_create(
    VkImageView *image_view,
    VkDevice log_dev,
    VkImage image,
    VkFormat format
) {
    VkImageViewCreateInfo create = {0};

    create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create.image = image;
    create.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create.format = format;
    create.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create.subresourceRange.baseMipLevel = 0;
    create.subresourceRange.levelCount = 1;
    create.subresourceRange.baseArrayLayer = 0;
    create.subresourceRange.layerCount = 0;

    VK_TRY(vkCreateImageView(log_dev, &create, nullptr, image_view));

    return RESULT_OK;
}
