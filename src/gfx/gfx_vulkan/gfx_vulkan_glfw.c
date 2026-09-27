#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <core/result.h>
#include <stdint.h>

void vulkan_platform_framebuffer_size_get(void *window, int32_t *width, int32_t *height) {
    glfwGetFramebufferSize(window, width, height);
}
const char **vulkan_platform_instance_extensions_get(uint32_t *count) {
    return glfwGetRequiredInstanceExtensions(count);
}

Result vulkan_platform_surface_create(VkInstance instance, void *window, VkSurfaceKHR *surface) {
    return glfwCreateWindowSurface(instance, window, NULL, surface) == VK_SUCCESS
        ? RESULT_OK : RESULT_ERR_GLFW;
}
