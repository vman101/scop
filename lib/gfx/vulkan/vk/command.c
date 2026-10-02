#include <utils/result_tools.h>
#include <utils/utils.h>
#include "vk.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

Result
vulkan_command_pool_create(VkDevice device, uint32_t family_index, VkCommandPool *command_pool) {
    VkCommandPoolCreateInfo pool_info = {0};

    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = family_index;

    VK_TRY(vkCreateCommandPool(device, &pool_info, NULL, command_pool));

    return RESULT_OK;
}

Result
vulkan_command_buffer_create(VkDevice device, VkCommandPool command_pool, VkCommandBuffer *command_buffer) {
    VkCommandBufferAllocateInfo buf_info = {0};
    buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buf_info.commandPool = command_pool;
    buf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buf_info.commandBufferCount = 1;

    VK_TRY(vkAllocateCommandBuffers(device, &buf_info, command_buffer));

    return RESULT_OK;
}
