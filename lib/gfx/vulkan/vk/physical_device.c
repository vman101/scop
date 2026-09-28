#include "core/core.h"
#include "vk.h"
#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include <string.h>

QueueFamilyIndices
vulkan_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {
        .present_family = QUEUE_NONE,
        .graphics_family = QUEUE_NONE,
    };
    VkQueueFamilyProperties *queue_props = nullptr;

    uint32_t count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    queue_props = alloc(sizeof(*queue_props) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_props);

    for (uint32_t i = 0; i < count; ++i) {
        if (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }
        VkBool32 present_supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_supported);
        if (present_supported == VK_TRUE) {
            indices.present_family = i;
        }
    }

    free(queue_props);

    return indices;
}

Result vulkan_check_device_extension_support(VkPhysicalDevice device, Array(CharPtr) *device_extensions) {
    size_t device_extensions_len = tda_size(device_extensions);
    uint32_t count = 0;
    Array(VkExtensionProperties) extentions = {0};
    Result res = RESULT_ERR_VULKAN;

    VK_TRY(vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL));
    if (count == 0) {
        fprintf(stderr, "This device does not support any extensions\n");
        goto done;
    }

    TRY(tda_create(&extentions, count));
    VK_TRY(vkEnumerateDeviceExtensionProperties(device, NULL, &count, (void *)tda_data(&extentions)));
    tda_size(&extentions) = count;

    for (size_t r = 0; r < device_extensions_len; r++) {
        const char *required = *tda_at(device_extensions, r);
        bool found = false;
        for (uint32_t a = 0; a < count; a++) {
            VkExtensionProperties *p = tda_at(&extentions, a);
            if (strcmp(required, p->extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Missing device extension: %s\n", required);
            goto done;
        }
    }
    res = RESULT_OK;

done:
    tda_destroy(&extentions);
    return res;
}

static bool
is_swapchain_adequate(SwapChainSupportDetails *details) {
    return (bool)(tda_size(&details->present_modes) > 0 && tda_size(&details->formats) > 0);
}

bool
vulkan_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, Array(CharPtr) *device_extensions) {
    // VkPhysicalDeviceProperties dev_properties = {0};
    // VkPhysicalDeviceFeatures dev_features = {0};
    // vkGetPhysicalDeviceProperties(device, &dev_properties);
    // vkGetPhysicalDeviceFeatures(device, &dev_features);

    QueueFamilyIndices indices = vulkan_device_find_queue_families(device, surface);
    Result extensions_supported = vulkan_check_device_extension_support(device, device_extensions);
    SwapChainSupportDetails details = {};
    Result swap_chain_res = vulkan_swapchain_support_query(device, surface, &details);
    const bool swapchain_adequate = is_swapchain_adequate(&details);
    tda_destroy(&details.formats);
    tda_destroy(&details.present_modes);


    return (bool)(indices.graphics_family != QUEUE_NONE
            && indices.present_family != QUEUE_NONE
            && swapchain_adequate
            && extensions_supported == RESULT_OK
            && swap_chain_res == RESULT_OK);
}

Result
vulkan_device_pick(VkInstance instance, VkSurfaceKHR surface, Array(CharPtr) *device_extensions, VkPhysicalDevice *device) {
    uint32_t device_count = 0;

    VK_TRY(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    if (device_count == 0) {
        fprintf(stderr, "No vulkan devices found\n");
        return RESULT_ERR_VULKAN;
    }

    VkPhysicalDevice *device_list = (VkPhysicalDevice *)alloc(sizeof(*device_list) * device_count);
    if (!device_list) {
        return RESULT_ERR_ALLOC;
    }
    VK_TRY(vkEnumeratePhysicalDevices(instance, &device_count, device_list));

    for (uint32_t i = 0; i < device_count; ++i) {
        if (vulkan_device_suitable(device_list[i], surface, device_extensions)) {
            *device = device_list[i];
            break ;
        }
    }

    if (*device == VK_NULL_HANDLE) {
        fprintf(stderr, "Couldn't find suitable Vulkan device\n");
        return RESULT_ERR_VULKAN;
    }

    free((void *)device_list);
    return RESULT_OK;
}
