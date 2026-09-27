#include "GLFW/glfw3.h"
#include <renderer/renderer.h>
#include <core/vertex.h>

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
};

const Vertex vertices[] = {
    {{ 0.0F, -0.5F }, {1.0F, 0.0F, 0.0F}},
    {{ 0.5F, -0.5F }, {0.0F, 1.0F, 0.0F}},
    {{ -0.5F, 0.5F }, {0.0F, 0.0F, 1.0F}},
};

int main(void) {
   Renderer renderer = {0};

   while (!glfwWindowShouldClose(renderer.window)) {
       glfwPollEvents();
       if (glfwGetKey(renderer.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
           glfwSetWindowShouldClose(renderer.window, GLFW_TRUE);
       }
   }
   return 0;
}
