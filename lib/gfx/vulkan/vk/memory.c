#include "vk.h"
#include <string.h>
#include <vulkan/vulkan_core.h>
#include <utils/result_tools.h>
#include <utils/utils.h>

Result
vulkan_memory_type_find(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, uint32_t *type) {
    VkPhysicalDeviceMemoryProperties mem_props = {0};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if (
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            *type = i;
            return RESULT_OK;
        }
    }

    return RESULT_ERR_VULKAN;
}

Result
vulkan_memory_allocate(VkDevice device, VkPhysicalDevice physical_device, VkMemoryRequirements mem_req, VkDeviceMemory *device_memory) {
    uint32_t mem_type_index;
    TRY(vulkan_memory_type_find(
        physical_device,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &mem_type_index
    ));
    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_index;

    VK_TRY(vkAllocateMemory(device, &alloc_info, NULL, device_memory));

    return RESULT_OK;
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
