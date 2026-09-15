#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#define VK_CHECK(call, ret) do { \
    VkResult r_ = (call); \
    if (r_ != VK_SUCCESS) { \
        fprintf(stderr, "%s:%d: %s failed (%d)\n", __FILE__, __LINE__, #call, r_); \
        return (ret); \
    } } while (0)


#define APP_CHECK(call, ret) do { \
    t_result r_ = (call); \
    if (r_ != SUCCESS) { \
        fprintf(stderr, "%s:%d: %s failed (%d)\n", __FILE__, __LINE__, #call, r_); \
        return (ret); \
    } } while (0)

typedef struct {
    VkInstance  vk;
    GLFWwindow  *window;
} t_app;

typedef enum {
    SUCCESS,
    FAILURE,
    COUNT,
} t_result;

void *
alloc(uint32_t size) {
    return calloc(1, size);
}

static void
error_cb(int code, const char *desc) {
    fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

t_result
vulkan_instance_property_count_get(uint32_t *ext_count) {

    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, ext_count, nullptr), FAILURE);

    return SUCCESS;
}

t_result
vulkan_instance_extension_properties_get(VkExtensionProperties **properties) {
    uint32_t ext_count = 0;
    vulkan_instance_property_count_get(&ext_count);

    VkExtensionProperties *const extentions = alloc(ext_count * sizeof(*extentions));
    if (!extentions) { return FAILURE; }

    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, nullptr, extentions), FAILURE);

    *properties = extentions;

    return SUCCESS;
}

t_result
vulkan_instance_create(const VkInstance *instance) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;
    create_info.enabledLayerCount = 0;

    VK_CHECK(vkCreateInstance(&create_info, nullptr, (VkInstance *)instance), FAILURE);
    return SUCCESS;
}

t_result
window_init(
    const GLFWwindow **window,
    const uint32_t width,
    const uint32_t height,
    const char *title
) {
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return EXIT_FAILURE;
    }
    printf("%s\n", glfwGetVersionString());
    printf("platform: %s\n",
          glfwGetPlatform() == GLFW_PLATFORM_X11 ? "X11" : "Wayland");

    if (!glfwVulkanSupported()) {
        fprintf(stderr, "Vulkan not supported\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *new_win = glfwCreateWindow((int32_t)width, (int32_t)height, title, nullptr, nullptr);
    if (!new_win) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    *window = new_win;
    return SUCCESS;
}

t_result
window_destroy(const GLFWwindow **window) {
    glfwDestroyWindow((GLFWwindow *)*window);
    *window = nullptr;
    return SUCCESS;
}

int
main() {
    const t_app app = {0};

    const int32_t WIDTH = 800;
    const int32_t HEIGHT = 600;

    glfwSetErrorCallback(error_cb);

    APP_CHECK(vulkan_instance_create(&app.vk), FAILURE);
    APP_CHECK(window_init((const GLFWwindow **)&app.window, WIDTH, HEIGHT, "Test"), FAILURE);

    while (!glfwWindowShouldClose(app.window)) {
        glfwPollEvents();
        if (glfwGetKey(app.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(app.window, GLFW_TRUE);
        }
    }

    window_destroy((const GLFWwindow **)&app.window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
