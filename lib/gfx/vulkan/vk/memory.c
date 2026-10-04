#include "vk.h"
#include <string.h>
#include <vulkan/vulkan_core.h>
#include <utils/result_tools.h>
#include <utils/utils.h>

Result
vulkan_memory_type_find(
    VkPhysicalDevice      physical_device,
    uint32_t              type_bits,
    VkMemoryPropertyFlags properties,
    uint32_t             *type
) {
    VkPhysicalDeviceMemoryProperties mem_props = {0};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_bits & (1U << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            *type = i;
            return RESULT_OK;
        }
    }
    return RESULT_ERR_VULKAN;
}

Result
vulkan_memory_write(
    VkDevice device,
    VkDeviceMemory memory,
    uint64_t offset,
    uint64_t size,
    const void *data
) {
    void *mapped = NULL;
    VK_TRY(vkMapMemory(device, memory, offset, size, 0, &mapped));
    memcpy(mapped, data, size);
    vkUnmapMemory(device, memory);
    return RESULT_OK;
}

void
vulkan_memory_free(VkDevice device, VkDeviceMemory memory) {
    vkFreeMemory(device, memory, NULL);
}
