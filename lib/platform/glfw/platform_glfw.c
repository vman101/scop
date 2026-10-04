#include <stdint.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "utils/utils.h"
#include <interface/mage_native_window.h>
#include <interface/mage_platform_keys.h>
#include <interface/mage_platform.h>

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
    PlatformWindowResizeCallbackFunc    resize_func;
    void                                *user_data;
    bool                                should_close;
    bool                                keys_prev[PLATFORM_KEY_COUNT];
    bool                                keys_now[PLATFORM_KEY_COUNT];
};

static void
error_cb(int code, const char *desc) {
    fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

void
platform_window_mouse_pos_get(PlatformWindow window, int32_t *x, int32_t *y) {
    double xpos;
    double ypos;
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    *x = (int32_t)floor(xpos);
    *y = (int32_t)floor(ypos);
}

bool
platform_window_should_close_get(PlatformWindow window) {
    return window->should_close;
}

void
platform_window_should_close_set(PlatformWindow window, bool should_close) {
    window->should_close = should_close;
}

void
platform_event_poll(PlatformWindow window) {
    memcpy(window->keys_prev, window->keys_now, sizeof(window->keys_prev));
    memset(window->keys_now, 0, sizeof(window->keys_now));
    (void)window;
    glfwPollEvents();
}

static int
platform_key_to_glfw(int key) {
    switch   (key) {
        case (PLATFORM_KEY_UNKNOWN):       return GLFW_KEY_UNKNOWN;
        case (PLATFORM_KEY_SPACE):         return GLFW_KEY_SPACE;
        case (PLATFORM_KEY_APOSTROPHE):    return GLFW_KEY_APOSTROPHE;
        case (PLATFORM_KEY_COMMA):         return GLFW_KEY_COMMA;
        case (PLATFORM_KEY_MINUS):         return GLFW_KEY_MINUS;
        case (PLATFORM_KEY_PERIOD):        return GLFW_KEY_PERIOD;
        case (PLATFORM_KEY_SLASH):         return GLFW_KEY_SLASH;
        case (PLATFORM_KEY_0):             return GLFW_KEY_0;
        case (PLATFORM_KEY_1):             return GLFW_KEY_1;
        case (PLATFORM_KEY_2):             return GLFW_KEY_2;
        case (PLATFORM_KEY_3):             return GLFW_KEY_3;
        case (PLATFORM_KEY_4):             return GLFW_KEY_4;
        case (PLATFORM_KEY_5):             return GLFW_KEY_5;
        case (PLATFORM_KEY_6):             return GLFW_KEY_6;
        case (PLATFORM_KEY_7):             return GLFW_KEY_7;
        case (PLATFORM_KEY_8):             return GLFW_KEY_8;
        case (PLATFORM_KEY_9):             return GLFW_KEY_9;
        case (PLATFORM_KEY_SEMICOLON):     return GLFW_KEY_SEMICOLON;
        case (PLATFORM_KEY_EQUAL):         return GLFW_KEY_EQUAL;
        case (PLATFORM_KEY_A):             return GLFW_KEY_A;
        case (PLATFORM_KEY_B):             return GLFW_KEY_B;
        case (PLATFORM_KEY_C):             return GLFW_KEY_C;
        case (PLATFORM_KEY_D):             return GLFW_KEY_D;
        case (PLATFORM_KEY_E):             return GLFW_KEY_E;
        case (PLATFORM_KEY_F):             return GLFW_KEY_F;
        case (PLATFORM_KEY_G):             return GLFW_KEY_G;
        case (PLATFORM_KEY_H):             return GLFW_KEY_H;
        case (PLATFORM_KEY_I):             return GLFW_KEY_I;
        case (PLATFORM_KEY_J):             return GLFW_KEY_J;
        case (PLATFORM_KEY_K):             return GLFW_KEY_K;
        case (PLATFORM_KEY_L):             return GLFW_KEY_L;
        case (PLATFORM_KEY_M):             return GLFW_KEY_M;
        case (PLATFORM_KEY_N):             return GLFW_KEY_N;
        case (PLATFORM_KEY_O):             return GLFW_KEY_O;
        case (PLATFORM_KEY_P):             return GLFW_KEY_P;
        case (PLATFORM_KEY_Q):             return GLFW_KEY_Q;
        case (PLATFORM_KEY_R):             return GLFW_KEY_R;
        case (PLATFORM_KEY_S):             return GLFW_KEY_S;
        case (PLATFORM_KEY_T):             return GLFW_KEY_T;
        case (PLATFORM_KEY_U):             return GLFW_KEY_U;
        case (PLATFORM_KEY_V):             return GLFW_KEY_V;
        case (PLATFORM_KEY_W):             return GLFW_KEY_W;
        case (PLATFORM_KEY_X):             return GLFW_KEY_X;
        case (PLATFORM_KEY_Y):             return GLFW_KEY_Y;
        case (PLATFORM_KEY_Z):             return GLFW_KEY_Z;
        case (PLATFORM_KEY_LEFT_BRACKET):  return GLFW_KEY_LEFT_BRACKET;
        case (PLATFORM_KEY_BACKSLASH):     return GLFW_KEY_BACKSLASH;
        case (PLATFORM_KEY_RIGHT_BRACKET): return GLFW_KEY_RIGHT_BRACKET;
        case (PLATFORM_KEY_GRAVE_ACCENT):  return GLFW_KEY_GRAVE_ACCENT;
        case (PLATFORM_KEY_WORLD_1):       return GLFW_KEY_WORLD_1;
        case (PLATFORM_KEY_WORLD_2):       return GLFW_KEY_WORLD_2;
        case (PLATFORM_KEY_ESCAPE):        return GLFW_KEY_ESCAPE;
        case (PLATFORM_KEY_ENTER):         return GLFW_KEY_ENTER;
        case (PLATFORM_KEY_TAB):           return GLFW_KEY_TAB;
        case (PLATFORM_KEY_BACKSPACE):     return GLFW_KEY_BACKSPACE;
        case (PLATFORM_KEY_INSERT):        return GLFW_KEY_INSERT;
        case (PLATFORM_KEY_DELETE):        return GLFW_KEY_DELETE;
        case (PLATFORM_KEY_RIGHT):         return GLFW_KEY_RIGHT;
        case (PLATFORM_KEY_LEFT):          return GLFW_KEY_LEFT;
        case (PLATFORM_KEY_DOWN):          return GLFW_KEY_DOWN;
        case (PLATFORM_KEY_UP):            return GLFW_KEY_UP;
        case (PLATFORM_KEY_PAGE_UP):       return GLFW_KEY_PAGE_UP;
        case (PLATFORM_KEY_PAGE_DOWN):     return GLFW_KEY_PAGE_DOWN;
        case (PLATFORM_KEY_HOME):          return GLFW_KEY_HOME;
        case (PLATFORM_KEY_END):           return GLFW_KEY_END;
        case (PLATFORM_KEY_CAPS_LOCK):     return GLFW_KEY_CAPS_LOCK;
        case (PLATFORM_KEY_SCROLL_LOCK):   return GLFW_KEY_SCROLL_LOCK;
        case (PLATFORM_KEY_NUM_LOCK):      return GLFW_KEY_NUM_LOCK;
        case (PLATFORM_KEY_PRINT_SCREEN):  return GLFW_KEY_PRINT_SCREEN;
        case (PLATFORM_KEY_PAUSE):         return GLFW_KEY_PAUSE;
        case (PLATFORM_KEY_F1):            return GLFW_KEY_F1;
        case (PLATFORM_KEY_F2):            return GLFW_KEY_F2;
        case (PLATFORM_KEY_F3):            return GLFW_KEY_F3;
        case (PLATFORM_KEY_F4):            return GLFW_KEY_F4;
        case (PLATFORM_KEY_F5):            return GLFW_KEY_F5;
        case (PLATFORM_KEY_F6):            return GLFW_KEY_F6;
        case (PLATFORM_KEY_F7):            return GLFW_KEY_F7;
        case (PLATFORM_KEY_F8):            return GLFW_KEY_F8;
        case (PLATFORM_KEY_F9):            return GLFW_KEY_F9;
        case (PLATFORM_KEY_F10):           return GLFW_KEY_F10;
        case (PLATFORM_KEY_F11):           return GLFW_KEY_F11;
        case (PLATFORM_KEY_F12):           return GLFW_KEY_F12;
        case (PLATFORM_KEY_F13):           return GLFW_KEY_F13;
        case (PLATFORM_KEY_F14):           return GLFW_KEY_F14;
        case (PLATFORM_KEY_F15):           return GLFW_KEY_F15;
        case (PLATFORM_KEY_F16):           return GLFW_KEY_F16;
        case (PLATFORM_KEY_F17):           return GLFW_KEY_F17;
        case (PLATFORM_KEY_F18):           return GLFW_KEY_F18;
        case (PLATFORM_KEY_F19):           return GLFW_KEY_F19;
        case (PLATFORM_KEY_F20):           return GLFW_KEY_F20;
        case (PLATFORM_KEY_F21):           return GLFW_KEY_F21;
        case (PLATFORM_KEY_F22):           return GLFW_KEY_F22;
        case (PLATFORM_KEY_F23):           return GLFW_KEY_F23;
        case (PLATFORM_KEY_F24):           return GLFW_KEY_F24;
        case (PLATFORM_KEY_F25):           return GLFW_KEY_F25;
        case (PLATFORM_KEY_KP_0):          return GLFW_KEY_KP_0;
        case (PLATFORM_KEY_KP_1):          return GLFW_KEY_KP_1;
        case (PLATFORM_KEY_KP_2):          return GLFW_KEY_KP_2;
        case (PLATFORM_KEY_KP_3):          return GLFW_KEY_KP_3;
        case (PLATFORM_KEY_KP_4):          return GLFW_KEY_KP_4;
        case (PLATFORM_KEY_KP_5):          return GLFW_KEY_KP_5;
        case (PLATFORM_KEY_KP_6):          return GLFW_KEY_KP_6;
        case (PLATFORM_KEY_KP_7):          return GLFW_KEY_KP_7;
        case (PLATFORM_KEY_KP_8):          return GLFW_KEY_KP_8;
        case (PLATFORM_KEY_KP_9):          return GLFW_KEY_KP_9;
        case (PLATFORM_KEY_KP_DECIMAL):    return GLFW_KEY_KP_DECIMAL;
        case (PLATFORM_KEY_KP_DIVIDE):     return GLFW_KEY_KP_DIVIDE;
        case (PLATFORM_KEY_KP_MULTIPLY):   return GLFW_KEY_KP_MULTIPLY;
        case (PLATFORM_KEY_KP_SUBTRACT):   return GLFW_KEY_KP_SUBTRACT;
        case (PLATFORM_KEY_KP_ADD):        return GLFW_KEY_KP_ADD;
        case (PLATFORM_KEY_KP_ENTER):      return GLFW_KEY_KP_ENTER;
        case (PLATFORM_KEY_KP_EQUAL):      return GLFW_KEY_KP_EQUAL;
        case (PLATFORM_KEY_LEFT_SHIFT):    return GLFW_KEY_LEFT_SHIFT;
        case (PLATFORM_KEY_LEFT_CONTROL):  return GLFW_KEY_LEFT_CONTROL;
        case (PLATFORM_KEY_LEFT_ALT):      return GLFW_KEY_LEFT_ALT;
        case (PLATFORM_KEY_LEFT_SUPER):    return GLFW_KEY_LEFT_SUPER;
        case (PLATFORM_KEY_RIGHT_SHIFT):   return GLFW_KEY_RIGHT_SHIFT;
        case (PLATFORM_KEY_RIGHT_CONTROL): return GLFW_KEY_RIGHT_CONTROL;
        case (PLATFORM_KEY_RIGHT_ALT):     return GLFW_KEY_RIGHT_ALT;
        case (PLATFORM_KEY_RIGHT_SUPER):   return GLFW_KEY_RIGHT_SUPER;
        default:                           return GLFW_KEY_UNKNOWN;
    }
}

static int
platform_mouse_button_to_glfw(int mouse_btn) {
    switch (mouse_btn) {
        case (PLATFORM_MOUSE_LEFT):   return GLFW_MOUSE_BUTTON_LEFT;
        case (PLATFORM_MOUSE_RIGHT):  return GLFW_MOUSE_BUTTON_RIGHT;
        case (PLATFORM_MOUSE_MIDDLE): return GLFW_MOUSE_BUTTON_MIDDLE;
        default:                      return -1;
    }
}

bool
platform_mouse_button_is_pressed(PlatformWindow window, int mouse_btn) {
    int glfw_mouse_btn = platform_mouse_button_to_glfw(mouse_btn);
    if (mouse_btn == -1) return false;
    return glfwGetMouseButton(window->handle, glfw_mouse_btn);
}

bool
platform_key_is_pressed(PlatformWindow window, int key) {
    if (key > PLATFORM_KEY_COUNT) { return false; }
    int glfw_key = platform_key_to_glfw(key);
    bool pressed = glfwGetKey(window->handle, glfw_key) == GLFW_PRESS;
    window->keys_now[key] = pressed;
    return pressed;
}

bool
platform_key_was_pressed(PlatformWindow window, int key) {
    bool pressed = platform_key_is_pressed(window, key);
    return pressed && !window->keys_prev[key];
}

Result
platform_window_create(
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

    GLFWwindow *new_win = glfwCreateWindow((int32_t)width, (int32_t)height, title, NULL, NULL);
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
    if (!window) return ;
    glfwDestroyWindow(window->handle);
    window->handle = NULL;
}

void
platform_window_resize_callback_internal(GLFWwindow *window, int32_t width, int32_t height) {
    PlatformWindow_T *platform_window = glfwGetWindowUserPointer(window);
    platform_window->resize_func(platform_window->user_data, width, height);
}

void
platform_window_resize_callback_set(PlatformWindow window, PlatformWindowResizeCallbackFunc func, void *user_data) {
    window->resize_func = func;
    window->user_data = user_data;
    glfwSetWindowUserPointer(window->handle, window);
    glfwSetFramebufferSizeCallback(window->handle, platform_window_resize_callback_internal);
}

void
platform_framebuffer_size_get(PlatformWindow window, int32_t *width, int32_t *height) {
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
#elif defined(__APPLE__)
    n.type = PLATFORM_WS_COCOA;
    n.cocoa = glfwGetCocoaWindow(window->handle);
#elif defined(__linux__)
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
