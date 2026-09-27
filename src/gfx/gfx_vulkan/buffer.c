#include "vk.h"
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

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
vulkan_buffer_memory_bind(VkDevice device, VkBuffer buffer, VkDeviceMemory device_memory) {
    VK_TRY(vkBindBufferMemory(device, buffer, device_memory, 0));
    return RESULT_OK;
}

VkMemoryRequirements
vulkan_buffer_memory_requirements_get(VkDevice device, VkBuffer buffer) {
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device, buffer, &mem_req);
    return mem_req;
}

Result
vulkan_buffer_create(VkDevice device, VkBufferCreateInfo *buffer_info, VkBuffer *buffer) {
    VK_TRY(vkCreateBuffer(device, buffer_info, nullptr, buffer));
    return RESULT_OK;
}
