#pragma once

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "core/core.h"
#include <core/array_types.h>

DECLARE_ARRAY(VkFramebuffer);
DECLARE_ARRAY(VkImageView);
DECLARE_ARRAY(VkImage);
DECLARE_ARRAY(VkPresentModeKHR);
DECLARE_ARRAY(VkSurfaceFormatKHR);
DECLARE_ARRAY(VkExtensionProperties);

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

/* GLFW_GLUE */
Result vulkan_platform_surface_create(VkInstance instance, void *window, VkSurfaceKHR *surface);

/* CONTEXT */
void vulkan_debug_messenger_populate(VkDebugUtilsMessengerCreateInfoEXT *create_info);
Result vulkan_debug_messenger_create(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT *create_info, VkDebugUtilsMessengerEXT *messenger);
void vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT mes);
void vulkan_instance_destroy(VkInstance instance);
Result vulkan_validation_layers_check(const char *validation_layers[], uint32_t layer_count);
Result vulkan_device_pick(VkInstance instance, VkSurfaceKHR surface, Array(CharPtr) *device_extensions, VkPhysicalDevice *device);
Result vulkan_logical_device_create(VkPhysicalDevice phys_device, VkSurfaceKHR surface, Array(CharPtr) *dev_extensions, VkDevice *device);
QueueFamilyIndices vulkan_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
Result vulkan_swapchain_support_query(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails *details);
Result vulkan_swapchain_info_create(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, QueueFamilyIndices *indices, VkSwapchainCreateInfoKHR *swapchain_info, VkFormat *swapchain_format, VkExtent2D *swapchain_extent);
Result vulkan_swapchain_image_format_query(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR *out);
Result vulkan_swapchain_image_view_create(VkDevice log_dev, VkImage image, VkFormat format, VkImageView *image_view);
Result vulkan_swapchain_image_views_create_from_image(VkDevice device, VkFormat format, Array(VkImage) *images, Array(VkImageView) *image_views);

/* GRAPHICS_PIPELINE */
Result vulkan_shader_module_create_from_file(VkDevice device, const char *filename, VkShaderModule *module);
Result vulkan_render_pass_create(VkDevice device, const VkFormat *swap_chain_image_format, VkRenderPass *render_pass);
Result vulkan_framebuffers_create(VkDevice device, VkRenderPass render_pass, VkExtent2D extent, Array(VkImageView) *image_views, Array(VkFramebuffer) *fbs);

/* FRAMES */

Result vulkan_command_pool_create(VkDevice device, uint32_t family_index, VkCommandPool *command_pool);
Result vulkan_command_buffer_create(VkDevice device, VkCommandPool command_pool, VkCommandBuffer *command_buffer);
Result vulkan_semaphore_create(VkDevice device, VkSemaphore *semaphore);
Result vulkan_fence_create(VkDevice device, VkFence *fence);

/* MEMORY */
Result vulkan_memory_allocate(VkDevice device, VkPhysicalDevice physical_device, VkMemoryRequirements mem_req, VkDeviceMemory *device_memory);
void vulkan_memory_free(VkDevice device, VkDeviceMemory memory);
Result vulkan_memory_write(VkDevice device, VkDeviceMemory memory, uint64_t offset, uint64_t size, const void *data);
Result vulkan_memory_type_find(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, uint32_t *type);

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
