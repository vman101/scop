#include "renderer.h"
#include "core/core.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

Result
renderer_window_init(Renderer *renderer, uint32_t width, uint32_t height) {
    return window_init(&renderer->window, width, height, "Test");
}

Result
renderer_init(Renderer *renderer) {
    uint32_t WIDTH  = 800;
    uint32_t HEIGHT = 600;
    const char *validation_layers[1] = {
        "VK_LAYER_KHRONOS_validation",
    };
    memset(renderer, 0, sizeof(*renderer));

    TRY(window_init(&renderer->window, WIDTH, HEIGHT, "Test"));
    TRY(renderer_vulkan_init(renderer, validation_layers, ARRAY_LEN(validation_layers)));

    return RESULT_OK;
}

