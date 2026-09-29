#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "utils/utils.h"
#include <interface/native_window.h>
#include <interface/platform.h>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3native.h>

struct PlatformWindow_T {
    GLFWwindow                          *handle;
    PlatformWindowResizeCallbackFunc    func;
    void                                *user_data;
    bool                                should_close;
};

static void
error_cb(int code, const char *desc) {
    fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

bool    platform_window_should_close_get(PlatformWindow window) {
    return window->should_close;
}
void    platform_window_should_close_set(PlatformWindow window, bool should_close) {
    window->should_close = should_close;
}

void    platform_event_poll(PlatformWindow window) {
    (void)window;
    glfwPollEvents();
}

static int platform_key_to_glfw(int key) {
    switch (key) {
        default:
            return GLFW_KEY_ESCAPE;
    }
}

bool    platform_key_is_pressed(PlatformWindow window, int key) {
    int glfw_key = platform_key_to_glfw(key);
    return glfwGetKey(window->handle, glfw_key) == GLFW_PRESS;
}

Result
platform_window_init(
    uint32_t width,
    uint32_t height,
    const char *title,
    PlatformWindow *window
) {
    PlatformWindow_T *platform_window = alloc(sizeof(*platform_window));
    glfwSetErrorCallback(error_cb);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return RESULT_ERR_GLFW;
    }
    printf("%s\n", glfwGetVersionString());
    printf("platform: %s\n", glfwGetPlatform() == GLFW_PLATFORM_X11 ? "X11" : "Wayland");

    if (!glfwVulkanSupported()) {
        fprintf(stderr, "Vulkan not supported\n");
        glfwTerminate();
        return RESULT_ERR_GLFW;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *new_win = glfwCreateWindow((int32_t)width, (int32_t)height, title, nullptr, nullptr);
    if (!new_win) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return RESULT_ERR_GLFW;
    }

    platform_window->handle = new_win;
    *window = platform_window;
    return RESULT_OK;
}

void
platform_window_destroy(PlatformWindow window) {
    glfwDestroyWindow(window->handle);
    window->handle = nullptr;
}

void
platform_window_resize_callback_internal(GLFWwindow *window, int32_t width, int32_t height) {
    PlatformWindow_T *platform_window = glfwGetWindowUserPointer(window);
    platform_window->func(platform_window->user_data, width, height);
}

void
platform_window_resize_callback_set(PlatformWindow window, PlatformWindowResizeCallbackFunc func, void *user_data) {
    window->func = func;
    window->user_data = user_data;
    glfwSetWindowUserPointer(window->handle, window);
    glfwSetFramebufferSizeCallback(window->handle, platform_window_resize_callback_internal);
}

void
platform_framebuffer_size_func(PlatformWindow window, int32_t *width, int32_t *height) {
    glfwGetFramebufferSize(window->handle, width, height);
}

const char **
platform_instance_extensions_get(PlatformWindow window, uint32_t *count) {
    (void)window;
    return glfwGetRequiredInstanceExtensions(count);
}

CoreNativeWindow
platform_native_window_get(PlatformWindow window) {
    CoreNativeWindow n = {0};

#ifdef _WIN32
    n.type = PLATFORM_WS_WIN32;
    n.win32 = glfwGetWin32Window(window->handle);
#elifdef __APPLE__
    n.type = PLATFORM_WS_COCOA;
    n.cocoa = glfwGetCocoaWindow(window->handle);
#elifdef __linux__
    switch (glfwGetPlatform()) {
        case GLFW_PLATFORM_WAYLAND:
            n.type = PLATFORM_WS_WAYLAND;
            n.wayland.display = glfwGetWaylandDisplay();
            n.wayland.surface = glfwGetWaylandWindow(window->handle);
            break;
        case GLFW_PLATFORM_X11:
            n.type = PLATFORM_WS_X11;
            n.x11.display = glfwGetX11Display();
            n.x11.window  = glfwGetX11Window(window->handle);
            break;
        default:
            break;
    }
#endif
    return n;
}
