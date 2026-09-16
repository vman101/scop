#include "scop.h"

Result
app_window_init(App *app, uint32_t width, uint32_t height) {
    return window_init(&app->window, width, height, "Test");
}

Result
app_vulkan_init(App *app, const char **validation_layers, const uint32_t layers_count) {
    VkInstanceCreateInfo create_info = {0};
    VkApplicationInfo app_info = {0};

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    TRY(vulkan_validation_layers_check(validation_layers, layers_count));
    create_info.enabledLayerCount = layers_count;
    create_info.ppEnabledLayerNames = validation_layers;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    DynamicArray da = {0};
    TRY(da_create(&da, sizeof(*glfw_extensions), 16));

    for (uint32_t i = 0; i < glfw_extension_count; ++i) {
        TRY(da_push(&da, (void *)&glfw_extensions[i]));
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_mes_info = {0};

    if (debug_mode) {
        const char *dbg_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        TRY(da_push(&da, (void *)&dbg_ext));
        vulkan_debug_messenger_populate(&debug_mes_info);
        create_info.pNext = &debug_mes_info;
    }

    create_info.enabledExtensionCount = da.size;
    create_info.ppEnabledExtensionNames = (const char **)da.data;
    create_info.enabledLayerCount = 0;
    create_info.enabledLayerCount = layers_count;
    create_info.ppEnabledLayerNames = validation_layers;

    VK_TRY(vkCreateInstance(&create_info, nullptr, &app->vk));

    if (debug_mode) {
        TRY(vulkan_debug_messenger_create(app->vk, &debug_mes_info, &app->mes));
    }

    da_destroy(&da);
    TRY(app_create_surface(app));
    TRY(vulkan_device_pick(app->vk, app->surface, &app->device));
    QueueFamilyIndices indices;
    TRY(vulkan_logical_device_create(app->device, app->surface, &app->log_dev, &indices));

    vkGetDeviceQueue(app->log_dev, indices.graphics_family, 0, &app->graphics_queue);
    vkGetDeviceQueue(app->log_dev, indices.present_family, 0, &app->present_queue);

    return RESULT_OK;
}

Result
app_init(App *app) {
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    const char *validation_layers[1] = {
        "VK_LAYER_KHRONOS_validation",
    };

    memset(app, 0, sizeof(*app));

    TRY(window_init(&app->window, WIDTH, HEIGHT, "Test"));
    TRY(app_vulkan_init(app, validation_layers, ARRAY_LEN(validation_layers)));

    return RESULT_OK;
}

void app_destroy(App *app) {
   if (app->mes) {
       vulkan_debug_messenger_destroy(app->vk, app->mes);
   }
   if (app->log_dev) {
       vkDestroyDevice(app->log_dev, nullptr);
   }
   if (app->surface) {
       vkDestroySurfaceKHR(app->vk, app->surface, nullptr);
   }
   if (app->vk) {
       vkDestroyInstance(app->vk, nullptr);
   }
   if (app->window) {
       glfwDestroyWindow(app->window);
   }
   glfwTerminate();
}
