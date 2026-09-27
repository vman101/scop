#include "vk.h"
#include <string.h>
#include <vulkan/vulkan_core.h>

static Result
vulkan_memory_type_find(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties, uint32_t *type) {
    VkPhysicalDeviceMemoryProperties mem_props = {0};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if (
            type_filter & (1 << i)
            && (mem_props.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            *type = i;
        }
    }
    return RESULT_ERR_VULKAN;
}

Result
vulkan_memory_allocate(VkDevice device, VkPhysicalDevice physical_device, VkMemoryRequirements mem_req, VkDeviceMemory *device_memory) {
    uint32_t mem_type_index;
    TRY(vulkan_memory_type_find(
        physical_device,
        mem_req.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &mem_type_index
    ));
    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_index;

    VK_TRY(vkAllocateMemory(device, &alloc_info, nullptr, device_memory));

    return RESULT_OK;
}

Result
vulkan_memory_fill(VkDevice device, VkDeviceMemory device_memory, size_t size, Array(uint32_t) *data) {
    void *memory_map = nullptr;

    VK_TRY(vkMapMemory(device, device_memory, 0, size, 0, &memory_map));
    memcpy(memory_map, tda_data(data), tda_size(data));
    vkUnmapMemory(device, device_memory);

    return RESULT_OK;
}

void
vulkan_memory_free(VkDevice device, VkDeviceMemory memory) {
    vkFreeMemory(device, memory, nullptr);
}
