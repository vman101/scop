#include "vk.h"
#include <vulkan/vulkan_core.h>
#include <string.h>

Result
vulkan_render_pass_create(VkDevice device, const VkFormat *swap_chain_image_format, VkRenderPass *render_pass) {
    VkAttachmentDescription color_attachment;
    memset(&color_attachment, 0, sizeof(color_attachment));
    color_attachment.format                    = *swap_chain_image_format;
    color_attachment.samples                   = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp                    = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp                   = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout             = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout               = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_attachment.flags                     = 0;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment            = 0;
    color_attachment_ref.layout                = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass               = {0};
    subpass.pipelineBindPoint                  = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount               = 1;
    subpass.pColorAttachments                  = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info    = {0};
    render_pass_info.sType                     = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount           = 1;
    render_pass_info.pAttachments              = &color_attachment;
    render_pass_info.subpassCount              = 1;
    render_pass_info.pSubpasses                = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VK_TRY(vkCreateRenderPass(device, &render_pass_info, nullptr, render_pass));

    return RESULT_OK;
}
