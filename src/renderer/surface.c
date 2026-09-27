#include "renderer.h"

Result
renderer_create_surface(Renderer *renderer) {
    VK_TRY(glfwCreateWindowSurface(renderer->ctx.instance, renderer->window, nullptr, &renderer->ctx.surface));
    return RESULT_OK;
}
