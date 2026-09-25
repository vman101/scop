#include "renderer.h"

Result
renderer_create_surface(Renderer *renderer) {
    VK_TRY(glfwCreateWindowSurface(renderer->vk, renderer->window, nullptr, &renderer->surface));
    return RESULT_OK;
}
