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

Result
vulkan_check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extension_count = 0;
    VK_TRY(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr));

    DynamicArray da = {0};
    TRY(da_create(&da, sizeof(VkExtensionProperties), extension_count));

    VK_TRY(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, (void *)&da.data));

    da_destroy(&da);
}

bool
vulkan_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    // VkPhysicalDeviceProperties dev_properties = {0};
    // VkPhysicalDeviceFeatures dev_features = {0};
    // vkGetPhysicalDeviceProperties(device, &dev_properties);
    // vkGetPhysicalDeviceFeatures(device, &dev_features);

    QueueFamilyIndices indices = vulkan_device_find_queue_families(device, surface);


    return (bool)(indices.graphics_family != QUEUE_NONE && indices.present_family != QUEUE_NONE);
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
        fprintf(stderr, "Couldn't find suitable Vulkan device");
        return RESULT_ERR_VULKAN;
    }

    return RESULT_OK;
}
