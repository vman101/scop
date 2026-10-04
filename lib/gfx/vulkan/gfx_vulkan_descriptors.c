#include "utils/result_tools.h"
#include "interface/mage_gfx.h"
#include <stddef.h>
#include <vulkan/vulkan_core.h>
#include "gfx_vulkan_internal.h"
#include "utils/utils.h"
#include <stdio.h>

Result
gfx_vulkan_descriptor_layout_init(GfxDevice dev) {
    VkDescriptorSetLayoutBinding bindings[] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
    };

    VkDescriptorSetLayoutCreateInfo set_layout_info = {0};
    set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = ARRAY_LEN(bindings);
    set_layout_info.pBindings = bindings;

    VkDescriptorSetLayout set_layout = {0};
    VK_TRY(vkCreateDescriptorSetLayout(dev->ctx.logical_device, &set_layout_info, NULL, &set_layout));
    dev->descriptors.layout = set_layout;
    return RESULT_OK;
}

Result
gfx_vulkan_descriptor_pool_create(GfxDevice dev) {
    VkDescriptorPoolSize pool_size = {0};
    pool_size.descriptorCount = GFX_GLOBAL_SLOT_COUNT;
    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes    = &pool_size;
    pool_info.maxSets       = GFX_GLOBAL_SLOT_COUNT;

    VK_TRY(vkCreateDescriptorPool(dev->ctx.logical_device, &pool_info, NULL, &dev->descriptors.pool));
    return RESULT_OK;
}

Result
gfx_vulkan_descriptor_set_create(GfxDevice dev) {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = dev->descriptors.pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &dev->descriptors.layout,
    };
    VK_TRY(vkAllocateDescriptorSets(dev->ctx.logical_device, &alloc_info, &dev->descriptors.set));
    return RESULT_OK;
}

void
gfx_global_buffer_set(GfxDevice dev, GfxGlobalSlot slot, GfxBuffer buf) {
    VkDescriptorBufferInfo buf_info = {
        .buffer = buf->handle,
        .offset = 0,
        .range  = VK_WHOLE_SIZE,
    };
    VkWriteDescriptorSet w = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = dev->descriptors.set,
        .dstBinding      = slot,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo     = &buf_info,
    };
    dev->global_buffers[slot] = buf;
    vkUpdateDescriptorSets(dev->ctx.logical_device, 1, &w, 0, NULL);
}
