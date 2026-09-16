#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define TRY_EXPECT(call, expect) do { \
    int32_t r_ = (int)(call); \
    if (r_ != (expect)) { \
        fprintf(stderr, "%s:%5d: %s failed (%d)\n", __FILE__, __LINE__, #call, r_); \
        return (r_); \
    } } while (0)

#define VK_TRY(call) do { \
    int32_t r_ = (call); \
    if (r_ != VK_SUCCESS) { \
        fprintf(stderr, "%s:%5d: %s failed (%d)\n", __FILE__, __LINE__, #call, r_); \
        return RESULT_ERR_VULKAN; \
    } } while (0)

#define TRY(call) do { \
    Result r_ = (call); \
    if (r_ != RESULT_OK) { \
        fprintf(stderr, "%s:%5d: %s failed (%d)\n", __FILE__, __LINE__, #call, r_); \
        return (r_); \
    } } while (0)

#define QUEUE_NONE UINT32_MAX

typedef struct {
    uint32_t graphics_family;
    uint32_t present_family;
} QueueFamilyIndices;

typedef struct {
    VkInstance                  vk;
    GLFWwindow                  *window;
    VkDebugUtilsMessengerEXT    mes;
    VkPhysicalDevice            device;
    VkDevice                    log_dev;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    VkSurfaceKHR                surface;
} App;

typedef struct {
    uint8_t     *data;
    size_t      member_size;
    size_t      cap;
    size_t      size;
} DynamicArray;

typedef enum {
    RESULT_OK,
    RESULT_ERR_ALLOC,
    RESULT_ERR_VULKAN,
    RESULT_ERR_GLFW,
    COUNT,
} Result;

extern uint32_t debug_mode;

void *alloc(uint32_t size);

Result da_create(DynamicArray *da, size_t member_size, size_t cap);
Result da_push(DynamicArray *da, const void *elem);
bool da_pop(DynamicArray *da, void *out);
void *da_get(DynamicArray *da, size_t index);
void da_destroy(DynamicArray *da);
void da_remove_index(DynamicArray *da, size_t index);

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
Result app_create_surface(App *app);



Result app_window_init(App *app, uint32_t width, uint32_t height);
Result app_vulkan_init(App *app, const char **validation_layers, uint32_t layers_count);
Result app_init(App *app);
void app_destroy(App *app);
