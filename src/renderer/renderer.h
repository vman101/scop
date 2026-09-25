#pragma once

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "core/core.h"
#include <core/vertex.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    VkInstance                  vk;
    GLFWwindow                  *window;
    VkDebugUtilsMessengerEXT    mes;
    VkPhysicalDevice            device;
    VkDevice                    log_dev;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkSurfaceKHR                surface;
    VkSwapchainKHR              swapchain;
    Array(VkImage)              swapchain_images;
    Array(VkImageView)          swapchain_image_views;
    VkFormat                    swapchain_image_format;
    VkExtent2D                  swapchain_extent;
    VkPipelineLayout            graphics_pipeline_layout;
    VkPipeline                  graphics_pipeline;
    Array(VkBuffer)             buffers;
    Array(VkDeviceSize)         offsets;
    Array(VkFramebuffer)        framebuffers;
    Array(VkDeviceMemory)       device_memory;
    VkRenderPass                render_pass;
    VkCommandPool               command_pool;
    VkCommandBuffer             command_buffer;
    VkSemaphore                 image_available_semaphore;
    VkSemaphore                 render_finished_semaphore;
    VkFence                     in_flight_fence;
} Renderer;

typedef struct {
    VkVertexInputAttributeDescription attrs[2];
} VertexAttrDescs;

extern uint32_t debug_mode;
extern const char *device_extensions[2];

Result window_init(GLFWwindow **window, uint32_t width, uint32_t height, const char *title);

void glfw_window_destroy(GLFWwindow **window);
void glfw_destroy(void);

void vulkan_debug_messenger_populate(VkDebugUtilsMessengerCreateInfoEXT *create_info);
Result vulkan_debug_messenger_create(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT *create_info, VkDebugUtilsMessengerEXT *messenger);
void vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT mes);
void vulkan_instance_destroy(VkInstance instance);
Result vulkan_validation_layers_check(const char *validation_layers[], uint32_t layer_count);
Result vulkan_device_pick(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *device);
Result vulkan_logical_device_create(VkPhysicalDevice phys_device, VkSurfaceKHR surface, VkDevice *device, QueueFamilyIndices *indices_export);
QueueFamilyIndices vulkan_device_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
Result vulkan_swapchain_support_query(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails *details);
Result vulkan_swapchain_info_create(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSwapchainCreateInfoKHR *swapchain_info, VkFormat *swapchain_format, VkExtent2D *swapchain_extent);
Result vulkan_swapchain_image_view_create(VkDevice log_dev, VkImage image, VkFormat format, VkImageView *image_view);
Result vulkan_swapchain_image_views_create_from_image(VkDevice device, VkFormat format, Array(VkImage) *images, Array(VkImageView) *image_views);

Result vulkan_graphics_pipeline_create(VkDevice device, VkRenderPass render_pass, VkPipeline *graphics_pipeline, VkPipelineLayout *graphics_pipeline_layout, VkExtent2D *extend);
Result vulkan_shader_module_create_from_file(VkDevice device, const char *filename, VkShaderModule *module);
Result vulkan_render_pass_create(VkDevice device, const VkFormat *swap_chain_image_format, VkRenderPass *render_pass);
Result vulkan_framebuffers_create(VkDevice device, Array(VkFramebuffer) *fbs, Array(VkImageView) *image_views, VkRenderPass render_pass, VkExtent2D extent);

Result vulkan_command_pool_create(VkDevice device, VkCommandPool *command_pool, QueueFamilyIndices *queue_indices);
Result vulkan_command_buffer_create(VkDevice device, VkCommandPool command_pool, VkCommandBuffer *command_buffer);

Result vulkan_semaphore_create(VkDevice device, VkSemaphore *semaphore);
Result vulkan_fence_create(VkDevice device, VkFence *fence);
Result vulkan_sync_objects_create(VkDevice device, Array(VkSemaphorePtr) *semaphores, Array(VkFencePtr) *fences);

Result
vulkan_command_buffer_record(
    VkCommandBuffer command_buffer,
    uint32_t image_index,
    VkRenderPass render_pass,
    VkPipeline graphics_pipeline,
    VkExtent2D extent,
    Array(VkBuffer) *buffers,
    Array(VkDeviceSize) *offsets,
    Array(VkFramebuffer) *framebuffers
);

Result renderer_create_surface(Renderer *renderer);
Result renderer_window_init(Renderer *renderer, uint32_t width, uint32_t height);
Result renderer_vulkan_init(Renderer *renderer, const char **validation_layers, uint32_t layers_count);
Result renderer_draw_frame(Renderer *renderer);
Result renderer_init(Renderer *renderer);
void renderer_destroy(Renderer *renderer);

VkVertexInputBindingDescription vulkan_vertex_input_bind_desc_get();
VertexAttrDescs vulkan_vertex_input_attr_desc_get();
