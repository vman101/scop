#include "GLFW/glfw3.h"
#include "gfx/gfx.h"
#include <renderer/renderer.h>
#include <core/vertex.h>
#include <stddef.h>

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
    GfxDevice dev;
    GLFWwindow *window;

    TRY(window_init(&window, 600, 600, "Test"));

    GfxDeviceDesc dev_desc = {
        .window = window,
        .app_name = "name",
        .debug_mode = true
    };
    TRY(gfx_device_create(&dev_desc, &dev));

    GfxVertexLayout vertex_layout = {
         .bindings = {
            {
                .stride = sizeof(*vertices),
                .attributes = {
                    { .location = 0, .format = GFX_FORMAT_FLOAT2, .offset = offsetof(Vertex, pos) },
                    { .location = 1, .format = GFX_FORMAT_FLOAT3, .offset = offsetof(Vertex, color) },
                },
                .attribute_count = 2
            }
        },
        .binding_count = 1
    };

    GfxPipelineDesc pipeline_desc = {
        .depth_test = false,
        vertex_layout,
    };

    TRY(gfx_pipeline_create(dev, &pipeline_desc));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        printf("der loopler\n");
    }

    gfx_device_destroy(dev);
    return 0;
}
