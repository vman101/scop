#pragma once

#include <assert.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <gfx/gfx.h>
#include <core/result.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
    GLFWwindow  *window;
    GfxDevice   *dev;
} Renderer;

Result  window_init(GLFWwindow **window, uint32_t width, uint32_t height, const char *title);
void    glfw_window_destroy(GLFWwindow **window);
void    glfw_destroy(void);
Result  renderer_create_surface(Renderer *renderer);
Result  renderer_window_init(Renderer *renderer, uint32_t width, uint32_t height);
Result  renderer_vulkan_init(Renderer *renderer, const char **validation_layers, uint32_t layers_count);
Result  renderer_draw_frame(Renderer *renderer);
Result  renderer_init(Renderer *renderer);
void    renderer_destroy(Renderer *renderer);
