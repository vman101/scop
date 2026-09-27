#pragma once

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "core/core.h"

DECLARE_ARRAY(uint32_t);
DECLARE_ARRAY(VkFence);
DECLARE_ARRAY_NAMED(VkFencePtr, VkFence *);
DECLARE_ARRAY(VkSemaphore);
DECLARE_ARRAY_NAMED(VkSemaphorePtr, VkSemaphore *);
DECLARE_ARRAY(VkFramebuffer);
DECLARE_ARRAY(VkImageView);
DECLARE_ARRAY_NAMED(VkImageViewPtr, VkImageView *);
DECLARE_ARRAY(VkImage);
DECLARE_ARRAY_NAMED(VkImagePtr, VkImage *);
DECLARE_ARRAY(VkPresentModeKHR);
DECLARE_ARRAY(VkSurfaceFormatKHR);
DECLARE_ARRAY(VkBuffer);
DECLARE_ARRAY(VkDeviceSize);
DECLARE_ARRAY(VkDeviceMemory);
DECLARE_ARRAY(VkVertexInputAttributeDescription);

#define QUEUE_NONE UINT32_MAX

typedef struct {
    uint32_t graphics_family;
    uint32_t present_family;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR    capabilities;
    Array(VkSurfaceFormatKHR)   formats;
    Array(VkPresentModeKHR)     present_modes;
} SwapChainSupportDetails;

typedef struct {
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    messenger;
    VkPhysicalDevice            physical_device;
    VkDevice                    logical_device;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkSurfaceKHR                surface;
} VkContext;

typedef struct {
    VkSwapchainKHR              handle;
    VkFormat                    image_format;
    VkExtent2D                  extent;
    Array(VkImage)              images;
    Array(VkImageView)          image_views;
    Array(VkFramebuffer)        framebuffers;
} Swapchain;

typedef struct {
    VkPipelineLayout            layout;
    VkPipeline                  handle;
} GraphicsPipeline;

typedef struct {
    VkBuffer                    vertices;
    VkDeviceSize                vertex_offset;
    VkBuffer                    indices;
    VkDeviceSize                indices_offset;
    uint32_t                    index_count;
} VertexBuffer;

typedef struct {
    VkCommandBuffer             command_buffer;
    VkSemaphore                 image_available_semaphore;
    VkSemaphore                 render_finished_semaphore;
    VkFence                     in_flight_fence;
} FrameData;

typedef struct {
    VkDeviceMemory  device_mem;
    size_t          size;
} MemoryBlock;

DECLARE_ARRAY(MemoryBlock);

typedef struct {
    uint32_t memoryTypeIndex;
    Array(MemoryBlock) blocks;
}  MemoryPool;

typedef struct {
    MemoryPool                  device_local;
} GpuMemory;

typedef struct {
    VkVertexInputAttributeDescription attrs[2];
} VertexAttrDescs;

extern uint32_t debug_mode;
extern const char *device_extensions[2];

/* GLFW_GLUE */
Result vulkan_platform_surface_create(VkInstance instance, void *window, VkSurfaceKHR *surface);
const char **vulkan_platform_instance_extensions_get(uint32_t *count);
void vulkan_platform_framebuffer_size_get(void *window, int32_t *width, int32_t *height);

/* CONTEXT */
void vulkan_debug_messenger_populate(VkDebugUtilsMessengerCreateInfoEXT *create_info);
Result vulkan_debug_messenger_create(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT *create_info, VkDebugUtilsMessengerEXT *messenger);
void vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT mes);
void vulkan_instance_destroy(VkInstance instance);
Result vulkan_validation_layers_check(const char *validation_layers[], uint32_t layer_count);
Result vulkan_device_pick(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *device);
Result vulkan_logical_device_create(VkPhysicalDevice phys_device, VkSurfaceKHR surface, VkDevice *device);
QueueFamilyIndices vulkan_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
Result vulkan_swapchain_support_query(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails *details);
Result vulkan_swapchain_info_create(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSwapchainCreateInfoKHR *swapchain_info, VkFormat *swapchain_format, VkExtent2D *swapchain_extent);
Result vulkan_swapchain_image_view_create(VkDevice log_dev, VkImage image, VkFormat format, VkImageView *image_view);
Result vulkan_swapchain_image_views_create_from_image(VkDevice device, VkFormat format, Array(VkImage) *images, Array(VkImageView) *image_views);

/* GRAPHICS_PIPELINE */

Result vulkan_graphics_pipeline_create(
    VkDevice device,
    VkRenderPass render_pass,
    VkVertexInputBindingDescription vertex_binding_desc[], uint32_t vertex_binding_desc_count,
    VkVertexInputAttributeDescription vertex_attr_desc[],
    uint32_t vertex_attr_desc_count,
    VkShaderModule vertex_shader,
    VkShaderModule fragment_shader,
    VkPipelineLayout graphics_pipeline_layout,
    VkPipeline *graphics_pipeline
);
Result vulkan_shader_module_create_from_file(VkDevice device, const char *filename, VkShaderModule *module);
Result vulkan_render_pass_create(VkDevice device, const VkFormat *swap_chain_image_format, VkRenderPass *render_pass);
Result vulkan_framebuffers_create(VkDevice device, VkRenderPass render_pass, VkExtent2D extent, Array(VkImageView) *image_views, Array(VkFramebuffer) *fbs);

/* FRAMES */

Result vulkan_command_pool_create(VkDevice device, uint32_t family_index, VkCommandPool *command_pool);
Result vulkan_command_buffer_create(VkDevice device, VkCommandPool command_pool, VkCommandBuffer *command_buffer);
Result vulkan_semaphore_create(VkDevice device, VkSemaphore *semaphore);
Result vulkan_fence_create(VkDevice device, VkFence *fence);
Result vulkan_sync_objects_create(VkDevice device, Array(VkSemaphorePtr) *semaphores, Array(VkFencePtr) *fences);

Result vulkan_command_buffer_record(
    VkCommandBuffer command_buffer,
    uint32_t image_index,
    VkRenderPass render_pass,
    VkPipeline graphics_pipeline,
    VkExtent2D extent,
    Array(VkBuffer) *buffers,
    Array(VkDeviceSize) *offsets,
    Array(VkFramebuffer) *framebuffers
);

/* MEMORY */
Result vulkan_memory_allocate(VkDevice device, VkPhysicalDevice physical_device, VkMemoryRequirements mem_req, VkDeviceMemory *device_memory);
void vulkan_memory_free(VkDevice device, VkDeviceMemory memory);
Result vulkan_memory_fill(VkDevice device, VkDeviceMemory device_memory, size_t size, Array(uint32_t) *data);

/* BUFFER */
Result vulkan_buffer_memory_bind(VkDevice device, VkBuffer buffer, VkDeviceMemory device_memory);
VkBufferCreateInfo vulkan_buffer_info_vertex_get(size_t vertex_count);
Result vulkan_vertex_buffer_create(
    VkDevice device,
    VkDeviceMemory device_memory,
    size_t vertex_count,
    const uint32_t *vertices,
    VkBuffer *buffer
);
Result vulkan_buffer_memory_fill(VkDevice device, VkDeviceMemory device_memory, size_t buffer_size, Array(uint32_t) *data);
Result vulkan_buffer_create(VkDevice device, VkBufferCreateInfo *buffer_info, VkBuffer *buffer);
VkMemoryRequirements vulkan_buffer_memory_requirements_get(VkDevice device, VkBuffer buffer);
