#include <stdint.h>
#define ALLOCATORS_IMPLEMENTATION
#include <utils/utils.h>
#include <utils/result_tools.h>
#include "gfx_vulkan_internal.h"

static bool
block_try_alloc(GpuBlock *block, uint64_t size, uint64_t align, GpuAllocation *out) {
    uint64_t offset;
    if (allocate(&block->alloc, size, align, &offset) != RESULT_OK) {
        return false;
    }
    *out = (GpuAllocation){
        .memory = block->device_mem,
        .offset = offset,
        .mapped = block->mapped ? (uint8_t *)block->mapped + offset : NULL,
    };
    return true;
}

static GpuPool *
gfx_pool_for(GfxDevice dev, GfxMemoryKind kind) {
    switch (kind) {
        case GFX_MEMORY_GPU:    return &dev->pool_gpu;
        case GFX_MEMORY_UPLOAD: return &dev->pool_upload;
        default:
            return NULL;
    }
}

static Result
gfx_memory_block_add(GfxDevice dev, GpuPool *pool) {
    GpuBlock block = {0};
    Result r = RESULT_OK;

    VkMemoryAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .memoryTypeIndex = pool->memory_type_index,
        .allocationSize = pool->block_size,
    };
    VK_TRY(vkAllocateMemory(dev->ctx.logical_device, &info, NULL, &block.device_mem));
    if (pool->host_visible) {
        VK_TRY_GOTO(r, fail, vkMapMemory(dev->ctx.logical_device, block.device_mem, 0, VK_WHOLE_SIZE, 0, &block.mapped));
    }
    linear_init(&block.alloc, pool->block_size);
    TRY_GOTO(r, fail, tda_push(&pool->blocks, &block));

    return RESULT_OK;
fail:
    vkFreeMemory(dev->ctx.logical_device, block.device_mem, NULL);
    return r;
}

Result
gfx_device_memory_request(GfxDevice dev, GfxMemoryKind mem_kind, uint64_t size, uint64_t align, GpuAllocation *out) {
    GpuPool *pool = gfx_pool_for(dev, mem_kind);
    if (!pool) {
        return RESULT_ERR_TODO;
    }
    if (size > pool->block_size) {
        return RESULT_OUT_OF_SPACE;
    }

    for (size_t i = 0; i < tda_size(&pool->blocks); ++i) {
        if (block_try_alloc(tda_at(&pool->blocks, i), size, align, out)) {
            return RESULT_OK;
        }
    }

    TRY(gfx_memory_block_add(dev, pool));
    GpuBlock *fresh = tda_at_safe(&pool->blocks, tda_size(&pool->blocks) - 1);
    return block_try_alloc(fresh, size, align, out) ? RESULT_OK : RESULT_OUT_OF_SPACE;
}

Result
gfx_memory_pool_create(GfxDevice dev, VkMemoryPropertyFlags props, uint64_t block_size, uint64_t block_count, GpuPool *pool) {
    *pool = (GpuPool){
        .block_size   = block_size,
        .host_visible = (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0,
    };
    TRY(tda_create(&pool->blocks, block_count));
    return vulkan_memory_type_find(dev->ctx.physical_device, UINT32_MAX, props, &pool->memory_type_index);
}

void
gfx_memory_pool_destroy(GfxDevice dev, GpuPool *pool) {
    for (size_t i = 0; i < tda_size(&pool->blocks); i++) {
        GpuBlock *b = tda_at(&pool->blocks, i);
        vkFreeMemory(dev->ctx.logical_device, b->device_mem, NULL);
    }
    tda_destroy(&pool->blocks);
}
