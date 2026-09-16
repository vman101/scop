#include "scop.h"

Result
app_create_surface(App *app) {
    VK_TRY(glfwCreateWindowSurface(app->vk, app->window, nullptr, &app->surface));
    return RESULT_OK;
}
