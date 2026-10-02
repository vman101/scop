#include "vk.h"
#include <vulkan/vulkan_core.h>
#include <utils/result_tools.h>

Result
vulkan_semaphore_create(VkDevice device, VkSemaphore *semaphore) {
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_TRY(vkCreateSemaphore(device, &semaphore_info, NULL, semaphore));
    return RESULT_OK;
}

Result
vulkan_fence_create(VkDevice device, VkFence *fence) {
    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_TRY(vkCreateFence(device, &fence_info, NULL, fence));
    return RESULT_OK;
}
