#include <utils/result_tools.h>
#include <utils/utils.h>
#include <utils/da.h>
#include "vk.h"
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

DECLARE_ARRAY(VkDeviceQueueCreateInfo);

Result
vulkan_logical_device_create(VkPhysicalDevice phys_device, VkSurfaceKHR surface, Array(CharPtr) *dev_extensions_arr, VkDevice *device) {
    QueueFamilyIndices indices = vulkan_device_find_queue_families(phys_device, surface);
    VkPhysicalDeviceFeatures device_features = {0};
    float queue_priority = 1.0F;

    const uint32_t families[] = { indices.graphics_family, indices.present_family };
    Array(uint32_t) da_families = {0};

    TRY(tda_create(&da_families, 10));
    for (size_t i = 0; i < ARRAY_LEN(families); ++i) {
        tda_push(&da_families, &families[i]);
    }

    tda_dedupe(&da_families);

    Array(VkDeviceQueueCreateInfo) da_queues = {0};

    for (uint32_t i = 0; i < tda_size(&da_families); ++i) {
        uint32_t family = *(uint32_t *)tda_get(&da_families, i);
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        tda_push(&da_queues, &queue_create_info);
    }

    VkPhysicalDeviceVulkan13Features f13 = {0};
    f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    f13.synchronization2 = VK_TRUE;

    const char **device_extensions = (const char **)tda_data(dev_extensions_arr);

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = tda_data(&da_queues);
    device_create_info.queueCreateInfoCount = tda_size(&da_queues);
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = tda_size(dev_extensions_arr);
    device_create_info.ppEnabledExtensionNames = device_extensions;
    device_create_info.pNext = &f13;

    VK_TRY(vkCreateDevice(phys_device, &device_create_info, NULL, device));

    tda_destroy(&da_families);
    tda_destroy(&da_queues);

    return RESULT_OK;
}
