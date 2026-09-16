#include "scop.h"

static void
error_cb(int code, const char *desc) {
    fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

Result
window_init(
    GLFWwindow **window,
    const uint32_t width,
    const uint32_t height,
    const char *title
) {
    glfwSetErrorCallback(error_cb);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return RESULT_ERR_GLFW;
    }
    printf("%s\n", glfwGetVersionString());
    printf("platform: %s\n",
          glfwGetPlatform() == GLFW_PLATFORM_X11 ? "X11" : "Wayland");

    if (!glfwVulkanSupported()) {
        fprintf(stderr, "Vulkan not supported\n");
        glfwTerminate();
        return RESULT_ERR_GLFW;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *new_win = glfwCreateWindow((int32_t)width, (int32_t)height, title, nullptr, nullptr);
    if (!new_win) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return RESULT_ERR_GLFW;
    }

    *window = new_win;
    return RESULT_OK;
}

void
glfw_window_destroy(GLFWwindow **window) {
    glfwDestroyWindow(*window);
    *window = nullptr;
}

void
glfw_destroy(void) {
    glfwTerminate();
}

