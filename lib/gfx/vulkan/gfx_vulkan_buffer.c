#include "core/result.h"
#include "gfx_vulkan_internal.h"
#include <vulkan/vulkan.h>
#include <core/core.h>
#include <string.h>
#include <stdlib.h>

static VkBufferUsageFlags
gfx_buffer_usage_to_vulkan(GfxBufferUsage u) {
    switch (u) {
        case GFX_BUFFER_VERTEX:  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case GFX_BUFFER_INDEX:   return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case GFX_BUFFER_UNIFORM: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    return 0;
}

Result
gfx_buffer_create(GfxDevice dev, GfxBufferDesc *desc, GfxBuffer *out) {
    Result r = RESULT_OK;
    uint64_t    buffer_bytes_size = desc->count * desc->member_size;

    GfxBuffer_T *gfx_buffer = alloc(sizeof(*gfx_buffer));
    gfx_buffer->bytes_size = buffer_bytes_size;
    gfx_buffer->count = desc->count;
    gfx_buffer->usage = desc->usage;
    gfx_buffer->member_size = desc->member_size;

    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_bytes_size;
    buffer_info.usage = gfx_buffer_usage_to_vulkan(desc->usage);

    TRY_GOTO(r, fail, vulkan_buffer_create(dev->ctx.logical_device, &buffer_info, &gfx_buffer->handle));

    VkMemoryRequirements mem_req = vulkan_buffer_memory_requirements_get(dev->ctx.logical_device, gfx_buffer->handle);
    TRY_GOTO(r, fail, gfx_device_memory_request(dev, desc->mem, mem_req.size, mem_req.alignment, &gfx_buffer->gpu_alloc));
    VK_TRY_GOTO(r, fail, vkBindBufferMemory(
        dev->ctx.logical_device,
        gfx_buffer->handle,
        gfx_buffer->gpu_alloc.memory,
        gfx_buffer->gpu_alloc.offset
    ));

    if (desc->data) {
        if (!gfx_buffer->gpu_alloc.mapped) {
            r = RESULT_ERR_TODO;
            goto fail;
        }
        memcpy(gfx_buffer->gpu_alloc.mapped, desc->data, buffer_bytes_size);
    }

    *out = gfx_buffer;
    return RESULT_OK;

fail:
    free(gfx_buffer);
    return r;
}

