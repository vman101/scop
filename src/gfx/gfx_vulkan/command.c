#include "vk.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

Result
vulkan_command_pool_create(VkDevice device, uint32_t family_index, VkCommandPool *command_pool) {
    VkCommandPoolCreateInfo pool_info = {0};

    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = family_index;

    VK_TRY(vkCreateCommandPool(device, &pool_info, nullptr, command_pool));

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

Result
vulkan_command_buffer_record(
    VkCommandBuffer command_buffer,
    uint32_t image_index,
    VkRenderPass render_pass,
    VkPipeline graphics_pipeline,
    VkExtent2D extent,
    Array(VkBuffer) *buffers,
    Array(VkDeviceSize) *offsets,
    Array(VkFramebuffer) *framebuffers
) {
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    VK_TRY(vkBeginCommandBuffer(command_buffer, &begin_info));
    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = *tda_at(framebuffers, image_index);
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = extent;

    VkClearValue clear_color = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    vkCmdBindVertexBuffers(command_buffer, 0, tda_size(buffers), tda_data(buffers), tda_data(offsets));

    vkCmdDraw(command_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(command_buffer);
    VK_TRY(vkEndCommandBuffer(command_buffer));

    return RESULT_OK;
}
