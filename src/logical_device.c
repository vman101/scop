#include "scop.h"
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static void
dedupe_array(DynamicArray *da) {
    for (size_t i = 0; i < da->size; ++i) {
        size_t j = i + 1;
        while (j < da->size) {
            if (memcmp(da_get(da, i), da_get(da, j), da->member_size) == 0) {
                da_remove_index(da, j);
            } else {
                ++j;
            }
        }
    }
}

Result
vulkan_logical_device_create(VkPhysicalDevice phys_device, VkSurfaceKHR surface, VkDevice *device, QueueFamilyIndices *indices_export) {
    QueueFamilyIndices indices = vulkan_device_find_queue_families(phys_device, surface);
    VkPhysicalDeviceFeatures device_features = {0};
    float queue_priority = 1.0F;

    const uint32_t families[] = { indices.graphics_family, indices.present_family };
    DynamicArray da_families = {0};

    TRY(da_create(&da_families, sizeof(uint32_t), 10));
    for (size_t i = 0; i < ARRAY_LEN(families); ++i) {
        da_push(&da_families, &families[i]);
    }

    dedupe_array(&da_families);

    DynamicArray da_queues = {0};
    da_create(&da_queues, sizeof(VkDeviceQueueCreateInfo), 8);

    for (uint32_t i = 0; i < da_families.size; ++i) {
        uint32_t family = *(uint32_t *)da_get(&da_families, i);
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        da_push(&da_queues, &queue_create_info);
    }


    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = da_queues.data;
    device_create_info.queueCreateInfoCount = da_queues.size;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = ARRAY_LEN(device_extensions);
    device_create_info.ppEnabledExtensionNames = device_extensions;

    VK_TRY(vkCreateDevice(phys_device, &device_create_info, nullptr, device));

    da_destroy(&da_families);
    da_destroy(&da_queues);

    *indices_export = indices;

    return RESULT_OK;
}
