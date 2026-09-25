#include "core/core.h"
#include <renderer/renderer.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

Result
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

VkBufferCreateInfo
vulkan_buffer_info_vertex_get(size_t vertex_count) {
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = vertex_count;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return buffer_info;
}

Result
vulkan_buffer_create(VkDevice device, VkBufferCreateInfo *buffer_info, VkBuffer *buffer) {
    VK_TRY(vkCreateBuffer(device, buffer_info, nullptr, buffer));
    return RESULT_OK;
}

VkMemoryRequirements
vulkan_buffer_memory_requirements_get(VkDevice device, VkBuffer buffer) {
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, buffer, &mem_req);
    return mem_req;
}

Result
vulkan_buffer_memory_allocate(VkDevice device, VkPhysicalDevice physical_device, VkBuffer buffer, VkDeviceMemory *device_memory) {
    uint32_t mem_type_index;
    VkMemoryRequirements mem_req = vulkan_buffer_memory_requirements_get(device, buffer);
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
vulkan_buffer_memory_bind(VkDevice device, VkBuffer buffer, VkDeviceMemory device_memory) {
    VK_TRY(vkBindBufferMemory(device, buffer, device_memory, 0));
    return RESULT_OK;
}

Result
vulkan_buffer_memory_fill(VkDevice device, VkDeviceMemory device_memory, size_t buffer_size, Array(uint32_t) *data) {
    void *memory_map = nullptr;

    VK_TRY(vkMapMemory(device, device_memory, 0, buffer_size, 0, &memory_map));
    memcpy(memory_map, tda_data(data), tda_size(data));
    vkUnmapMemory(device, device_memory);

    return RESULT_OK;
}

Result
vulkan_vertex_buffer_create(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkDeviceMemory *device_memory,
    VkBuffer *buffer,
    Array(Vertex) *vertices
) {
    VkBufferCreateInfo buffer_info = vulkan_buffer_info_vertex_get(tda_size(vertices));
    TRY(vulkan_buffer_create(device, &buffer_info, buffer));

    TRY(vulkan_buffer_memory_allocate(device, physical_device, *buffer, device_memory));
    TRY(vulkan_buffer_memory_bind(device, *buffer, *device_memory));
    TRY(vulkan_buffer_memory_fill(device, *device_memory, buffer_info.size, (Array(uint32_t) *)vertices));
    return RESULT_OK;
}
