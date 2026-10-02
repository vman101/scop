#include "interface/mage_math.h"
#include "interface/mage_gfx.h"
#include "gfx_vulkan_internal.h"
#include <vulkan/vulkan_core.h>
#include <utils/utils.h>
#include <utils/result_tools.h>

void
gfx_push_constant_float(GfxFrame frame, float f) {
    vkCmdPushConstants(frame->cmd, frame->dev->graphics_pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &f);
}

void
gfx_push_constant_mat4(GfxFrame frame, Mat4 m) {
    vkCmdPushConstants(frame->cmd, frame->dev->graphics_pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m), &m);
}

Result
gfx_frame_begin(GfxDevice dev, GfxFrame *out) {
    if (dev->swapchain_dirty) {
        TRY(gfx_swapchain_recreate(dev));
        if (dev->swapchain_dirty) {
            return RESULT_OK;
        }
    }

    *out = NULL;
    FrameData *fd = &dev->frames[dev->current_frame];
    VkDevice device = dev->ctx.logical_device;

    VK_TRY(vkWaitForFences(device, 1, &fd->in_flight_fence, VK_TRUE, UINT64_MAX));

    uint32_t image_index;
    VkResult res = vkAcquireNextImageKHR(
        device,
        dev->swapchain.handle,
        UINT64_MAX,
        fd->image_available,
        VK_NULL_HANDLE,
        &image_index
    );

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        TRY(gfx_swapchain_recreate(dev));
        return RESULT_OK;
    }

    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        return RESULT_ERR_VULKAN;
    }

    VK_TRY(vkResetFences(device, 1, &fd->in_flight_fence));
    VK_TRY(vkResetCommandPool(device, fd->command_pool, 0));
    VkCommandBufferBeginInfo begin = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_TRY(vkBeginCommandBuffer(fd->command_buffer, &begin));

    dev->frame = (GfxFrame_T){ .dev = dev, .data = fd, .cmd = fd->command_buffer, .image_index = image_index };
    *out = &dev->frame;
    return RESULT_OK;
}

void
gfx_pass_begin(GfxFrame frame, const float clear[4]) {
    GfxDevice dev = frame->dev;
    VkExtent2D extent = dev->swapchain.extent;

    VkClearValue clear_value = {
        .color = {
            .float32 = { clear[0], clear[1], clear[2], clear[3] },
        },
    };

    VkRenderPassBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = dev->render_pass,
        .renderArea = { .extent = extent},
        .framebuffer = *(VkFramebuffer *)tda_at_safe(&dev->swapchain.framebuffers, frame->image_index),
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };

    vkCmdBeginRenderPass(frame->cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = { .width = (float)extent.width, .height = (float)extent.height };
    VkRect2D scissor = { .extent = extent };
    vkCmdSetViewport(frame->cmd, 0, 1, &viewport);
    vkCmdSetScissor(frame->cmd, 0, 1, &scissor);
}

void
gfx_bind_pipeline(GfxFrame f, GfxPipeline p) {
    vkCmdBindPipeline(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->handle);
}

void
gfx_draw(GfxFrame frame, GfxBuffer vertices) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(frame->cmd, 0, 1, &vertices->handle, &offset);
    vkCmdDraw(frame->cmd, vertices->count, 1, 0, 0);
}

void
gfx_draw_indexed(GfxFrame frame, GfxBuffer vertices, GfxBuffer indices) {
    VkDeviceSize offset = 0;
    VkIndexType type = indices->member_size == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    vkCmdBindVertexBuffers(frame->cmd, 0, 1, &vertices->handle, &offset);
    vkCmdBindIndexBuffer(frame->cmd, indices->handle, 0, type);
    vkCmdDrawIndexed(frame->cmd, indices->count, 1, 0, 0, 0);
}

void
gfx_pass_end(GfxFrame frame) {
    vkCmdEndRenderPass(frame->cmd);
}

void
gfx_resize(GfxDevice_T *dev, int32_t width, int32_t height) {
    dev->width = width;
    dev->height = height;
    dev->swapchain_dirty = true;
}

Result
gfx_frame_end(GfxFrame frame) {
    GfxDevice_T *dev = frame->dev;
    FrameData *fd = frame->data;

    VK_TRY(vkEndCommandBuffer(frame->cmd));
    VkSemaphore *render_finished = tda_at_safe(&dev->swapchain.render_finished, frame->image_index);
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &fd->image_available,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame->cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = render_finished,
    };

    VK_TRY(vkQueueSubmit(dev->ctx.graphics_queue, 1, &submit, fd->in_flight_fence));

    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = render_finished,
        .swapchainCount = 1,
        .pSwapchains = &dev->swapchain.handle,
        .pImageIndices = &frame->image_index,
    };
    VkResult res = vkQueuePresentKHR(dev->ctx.present_queue, &present);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        TRY(gfx_swapchain_recreate(dev));
    } else if (res != VK_SUCCESS) {
        return RESULT_ERR_VULKAN;
    }

    dev->current_frame = (dev->current_frame + 1) % GFX_FRAME_COUNT;
    return RESULT_OK;
}
