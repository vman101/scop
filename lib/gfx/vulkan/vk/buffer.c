#include <utils/result_tools.h>
#include <utils/utils.h>
#include "vk.h"
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

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
    VK_TRY(vkCreateBuffer(device, buffer_info, NULL, buffer));
    return RESULT_OK;
}
