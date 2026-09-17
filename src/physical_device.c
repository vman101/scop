#include "scop.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

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

Result vulkan_check_device_extension_support(VkPhysicalDevice device) {
    size_t device_extensions_len = ARRAY_LEN(device_extensions);
    uint32_t count = 0;
    DynamicArray da = {0};
    Result res = RESULT_ERR_VULKAN;

    VK_TRY(vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL));
    if (count == 0) {
        fprintf(stderr, "This device does not support any extensions\n");
        goto done;
    }

    TRY(da_create(&da, sizeof(VkExtensionProperties), count));
    VK_TRY(vkEnumerateDeviceExtensionProperties(device, NULL, &count, (void *)da.data));
    da.size = count;

    for (size_t r = 0; r < device_extensions_len; r++) {
        bool found = false;
        for (size_t a = 0; a < da.size; a++) {
            VkExtensionProperties *p = da_get(&da, a);
            if (strcmp(device_extensions[r], p->extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Missing device extension: %s\n", device_extensions[r]);
            goto done;
        }
    }
    res = RESULT_OK;

done:
    da_destroy(&da);
    return res;
}

static bool
swap_chain_adequate(SwapChainSupportDetails *details) {
    return (bool)(details->present_modes.size > 0 && details->formats.size > 0);
}

bool
vulkan_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    // VkPhysicalDeviceProperties dev_properties = {0};
    // VkPhysicalDeviceFeatures dev_features = {0};
    // vkGetPhysicalDeviceProperties(device, &dev_properties);
    // vkGetPhysicalDeviceFeatures(device, &dev_features);

    QueueFamilyIndices indices = vulkan_device_find_queue_families(device, surface);
    Result extensions_supported = vulkan_check_device_extension_support(device);
    SwapChainSupportDetails details = {};
    Result swap_chain_res = vulkan_swapchain_support_query(device, surface, &details);


    return (bool)(indices.graphics_family != QUEUE_NONE
            && indices.present_family != QUEUE_NONE
            && swap_chain_adequate(&details)
            && extensions_supported == RESULT_OK
            && swap_chain_res == RESULT_OK);
}

Result vulkan_device_pick(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *device) {
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
        if (vulkan_device_suitable(device_list[i], surface)) {
            *device = device_list[i];
            break ;
        }
    }

    if (*device == VK_NULL_HANDLE) {
        fprintf(stderr, "Couldn't find suitable Vulkan device\n");
        return RESULT_ERR_VULKAN;
    }

    return RESULT_OK;
}
