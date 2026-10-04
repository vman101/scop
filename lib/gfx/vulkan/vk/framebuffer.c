#include <utils/result_tools.h>
#include <utils/utils.h>
#include "interface/mage_result.h"
#include "utils/da.h"
#include "vk.h"
#include <vulkan/vulkan_core.h>

Result
vulkan_framebuffers_create(
    VkDevice device,
    VkRenderPass render_pass,
    VkExtent2D extent,
    VkImageView depth_view,
    Array(VkImageView) *image_views,
    Array(VkFramebuffer) *out
) {
    Array(VkFramebuffer) fbs;
    Result r = RESULT_OK;
    TRY(tda_create(&fbs, tda_size(image_views)));

    for (size_t i = 0; i < tda_size(image_views); i++) {
        VkImageView *v = tda_at(image_views, i);
        if (!v) {
            r = RESULT_ERR_NOT_FOUND;
            goto fail;
        }
        VkImageView attachments[] = {
            *v,
            depth_view,
        };
        VkFramebufferCreateInfo fb_info = {0};
        fb_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass              = render_pass;
        fb_info.attachmentCount         = ARRAY_LEN(attachments);
        fb_info.pAttachments            = attachments;
        fb_info.width                   = extent.width;
        fb_info.height                  = extent.height;
        fb_info.layers                  = 1;
        VkFramebuffer fb;
        VK_TRY_GOTO(r, fail, vkCreateFramebuffer(device, &fb_info, NULL, &fb));
        TRY_GOTO(r, fail, tda_push(&fbs, (void *)&fb));
    }
    r = RESULT_OK;
    MOVE(out, fbs);
fail:
    tda_destroy(&fbs);
    return r;
}
