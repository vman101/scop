#include "scop.h"
#include <vulkan/vulkan_core.h>

Result
vulkan_framebuffers_create(
    VkDevice device,
    Array(VkFramebuffer) *fbs,
    Array(VkImageView) *image_views,
    VkRenderPass render_pass,
    VkExtent2D extent
) {
    TRY(tda_create(fbs, tda_size(image_views)));

    for (size_t i = 0; i < tda_size(image_views); i++) {
        VkImageView attachments[] = {
            *tda_at(image_views, i),
        };
        VkFramebufferCreateInfo fb_info = {0};
        fb_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass              = render_pass;
        fb_info.attachmentCount         = 1;
        fb_info.pAttachments            = attachments;
        fb_info.width                   = extent.width;
        fb_info.height                  = extent.height;
        fb_info.layers                  = 1;
        VK_TRY(vkCreateFramebuffer(device, &fb_info, nullptr, tda_at(fbs, i)));
        tda_size(fbs)++;
    }
    return RESULT_OK;
}
