#include "renderer.h"
#include <core/result.h>
#include <core/core.h>

Result
vulkan_draw_begin(VkContext *ctx, FrameData *frame) {
    VK_TRY(vkWaitForFences(ctx->logical_device, 1, &frame->in_flight_fence, VK_TRUE, UINT64_MAX));
    VK_TRY(vkResetFences(ctx->logical_device, 1, &frame->in_flight_fence));
    return RESULT_OK;
}

Result
renderer_draw_frame(Renderer *renderer) {
    TRY(vulkan_draw_begin(&renderer->ctx, &renderer->frames));

    uint32_t image_index;
    VK_TRY(vkAcquireNextImageKHR(
        renderer->ctx.logical_device,
        renderer->swapchain.handle,
        UINT64_MAX,
        renderer->frames.image_available_semaphore,
        VK_NULL_HANDLE,
        &image_index
    ));

    VK_TRY(vkResetCommandBuffer(renderer->frames.command_buffer, 0));
    // TRY(vulkan_command_buffer_record(
    //     renderer->frames.command_buffer,
    //     image_index,
    //     renderer->graphics_pipeline.render_pass,
    //     renderer->graphics_pipeline.handle,
    //     renderer->swapchain.extent,
    //     &renderer->buffers,
    //     &renderer->offsets,
    //     &renderer->framebuffers
    // ));

    VkSubmitInfo submit_info           = {0};
    submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore wait_semaphores[]      = {renderer->frames.image_available_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};

    assert(ARRAY_LEN(wait_semaphores) == ARRAY_LEN(wait_stages));

    submit_info.waitSemaphoreCount   = ARRAY_LEN(wait_semaphores);
    submit_info.pWaitSemaphores      = wait_semaphores;
    submit_info.pWaitDstStageMask    = wait_stages;

    VkSemaphore signal_semaphores[]  = {renderer->frames.render_finished_semaphore};
    submit_info.signalSemaphoreCount = ARRAY_LEN(signal_semaphores);
    submit_info.pSignalSemaphores    = signal_semaphores;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &renderer->frames.command_buffer;

    VK_TRY(vkQueueSubmit(renderer->ctx.graphics_queue, 1, &submit_info, renderer->frames.in_flight_fence));

    VkPresentInfoKHR present_info   = {0};
    VkSwapchainKHR swapchains[]     = {renderer->swapchain.handle};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = ARRAY_LEN(signal_semaphores);
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = ARRAY_LEN(swapchains);
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = nullptr;


    VK_TRY(vkQueuePresentKHR(renderer->ctx.present_queue, &present_info));

    return RESULT_OK;
}
