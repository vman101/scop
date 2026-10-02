#pragma once

#include <interface/gfx.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <utils/allocators.h>
#include <vulkan/vulkan_core.h>
#include "interface/native_window.h"
#include "vk/vk.h"

#define GPU_POOL_GPU_BLOCK_SIZE (512ULL * 1024 * 1024)
#define GPU_BLOCK_COUNT 8
#define GFX_FRAME_COUNT 2

typedef struct GpuBlock GpuBlock;

DECLARE_ARRAY(GpuBlock);
DECLARE_ARRAY(VkSemaphore);

struct GpuBlock {
    VkDeviceMemory  device_mem;
    size_t          size;
    void            *mapped;
    LinearAllocator alloc;
};

typedef struct {
    uint32_t            memory_type_index;
    VkDeviceSize        block_size;
    bool                host_visible;
    Array(GpuBlock)  blocks;
} GpuPool;

typedef struct {
    GpuPool  device_local;
} GpuMemory;

typedef struct {
    VkDeviceMemory  memory;
    uint64_t        offset;
    void            *mapped;
} GpuAllocation;

typedef struct {
    VkVertexInputAttributeDescription attrs[2];
} VertexAttrDescs;


typedef struct {
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    messenger;
    VkPhysicalDevice            physical_device;
    VkDevice                    logical_device;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkSurfaceKHR                surface;
    VkSurfaceFormatKHR          surface_format;
    QueueFamilyIndices          indices;
} VkContext;

typedef struct {
    VkCommandBuffer             command_buffer;
    VkCommandPool               command_pool;
    VkSemaphore                 image_available;
    VkFence                     in_flight_fence;
} FrameData;

typedef struct {
    VkSwapchainKHR              handle;
    VkFormat                    image_format;
    VkExtent2D                  extent;
    Array(VkImage)              images;
    Array(VkImageView)          image_views;
    Array(VkFramebuffer)        framebuffers;
    Array(VkSemaphore)          render_finished;
} Swapchain;

typedef struct {
    VkPipelineLayout            layout;
    VkPipeline                  handle;
} GraphicsPipeline;

struct GfxFrame_T {
    GfxDevice           dev;
    FrameData           *data;
    VkCommandBuffer     cmd;
    uint32_t            image_index;
};

struct GfxDevice_T {
    VkContext       ctx;
    Swapchain       swapchain;
    FrameData       frames[GFX_FRAME_COUNT];
    VkCommandPool   command_pool;
    VkRenderPass    render_pass;
    GpuPool         pool_gpu;
    GpuPool         pool_upload;
    uint32_t        current_frame;
    GfxPipeline     graphics_pipeline;
    GfxFrame_T      frame;
    uint32_t        width;
    uint32_t        height;
    bool            swapchain_dirty;
    bool            debug_mode;
};

DECLARE_ARRAY(GfxVertexLayout_T);

struct GfxPipelineDesc {
    const char                  *vertex_shader_path;
    const char                  *fragment_shader_path;
    bool                        depth_Test;
    Array(GfxVertexLayout_T)    vertex_desc;
};

typedef struct {
    const char **validation_layers;
    uint32_t layers_count;
    const CoreNativeWindow *window;
} VkContextCreateInfo;

struct GfxPipeline_T {
    VkPipeline handle;
    VkPipelineLayout layout;
};

struct GfxBuffer_T {
    VkBuffer        handle;
    uint64_t        bytes_size;
    GfxBufferUsage  usage;
    GpuAllocation   gpu_alloc;
    uint64_t        count;
    uint64_t        member_size;
};

/* SWAPCHAIN */
Result gfx_swapchain_init(GfxDevice_T *dev, uint32_t width, uint32_t height);
Result gfx_swapchain_recreate(GfxDevice dev);
void gfx_swapchain_destroy(GfxDevice_T *dev);
/* MEMORY */
Result gfx_memory_pool_init(GfxDevice dev, VkMemoryPropertyFlags props, uint64_t block_size, uint64_t block_count, GpuPool *pool);
Result gfx_device_memory_request(GfxDevice dev, GfxMemoryKind mem_kind, uint64_t size, uint64_t align, GpuAllocation *out);
