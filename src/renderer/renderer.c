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

void renderer_vulkan_loader_version_log(void) {
    uint32_t inst;
    vkEnumerateInstanceVersion(&inst);

    printf("Vulkan loader version: %u\n", inst);
}

void renderer_physical_device_properties_log(Renderer *renderer) {
    VkPhysicalDeviceProperties p;
    vkGetPhysicalDeviceProperties(renderer->ctx.physical_device, &p);       // p.apiVersion is the device version

    printf("GPU vulkan version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(p.apiVersion),
        VK_API_VERSION_MINOR(p.apiVersion),
        VK_API_VERSION_PATCH(p.apiVersion)
    );
}

Result
gfx_device_create(Renderer *renderer, const char **validation_layers, const uint32_t layers_count) {
    VkInstanceCreateInfo create_info    = {0};
    VkApplicationInfo renderer_info     = {0};

    renderer_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    renderer_info.pApplicationName   = "Hello Triangle";
    renderer_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    renderer_info.pEngineName        = "No Engine";
    renderer_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    renderer_info.apiVersion         = VK_API_VERSION_1_3;

    create_info.sType               = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo    = &renderer_info;
    create_info.enabledLayerCount   = layers_count;
    create_info.ppEnabledLayerNames = validation_layers;
    TRY(vulkan_validation_layers_check(validation_layers, layers_count));

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions  = nullptr;

    renderer_vulkan_loader_version_log();

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

    create_info.enabledExtensionCount   = da.size;
    create_info.ppEnabledExtensionNames = (const char **)da.data;
    create_info.enabledLayerCount       = 0;
    create_info.enabledLayerCount       = layers_count;
    create_info.ppEnabledLayerNames     = validation_layers;

    VK_TRY(vkCreateInstance(&create_info, nullptr, &renderer->ctx.instance));

    if (debug_mode) {
        TRY(vulkan_debug_messenger_create(renderer->ctx.instance, &debug_mes_info, &renderer->ctx.messenger));
    }

    da_destroy(&da);
    TRY(renderer_create_surface(renderer));
    TRY(vulkan_device_pick(renderer->ctx.instance, renderer->ctx.surface, &renderer->ctx.physical_device));
    QueueFamilyIndices indices;
    TRY(vulkan_logical_device_create(renderer->ctx.physical_device, renderer->ctx.surface, &renderer->ctx.logical_device, &indices));

    renderer_physical_device_properties_log(renderer);

    vkGetDeviceQueue(renderer->ctx.logical_device, indices.graphics_family, 0, &renderer->ctx.graphics_queue);
    vkGetDeviceQueue(renderer->ctx.logical_device, indices.present_family, 0, &renderer->ctx.present_queue);

    VkSwapchainCreateInfoKHR swapchain_info;
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(renderer->window, &width, &height);
    TRY(vulkan_swapchain_info_create(
        renderer->ctx.physical_device,
        renderer->ctx.surface,
        width,
        height,
        &swapchain_info,
        &renderer->swapchain.image_format,
        &renderer->swapchain.extent
    ));
    VK_TRY(vkCreateSwapchainKHR(renderer->ctx.logical_device, &swapchain_info, nullptr, &renderer->swapchain.handle));

    uint32_t image_count = 0;
    VK_TRY(vkGetSwapchainImagesKHR(renderer->ctx.logical_device, renderer->swapchain.handle, &image_count, nullptr));
    TRY(tda_create(&renderer->swapchain.images, image_count));
    VK_TRY(vkGetSwapchainImagesKHR(
        renderer->ctx.logical_device,
        renderer->swapchain.handle,
        &image_count,
        tda_data(&renderer->swapchain.images)
    ));
    tda_size(&renderer->swapchain.images) = image_count;

    TRY(vulkan_swapchain_image_views_create_from_image(renderer->ctx.logical_device, renderer->swapchain.image_format, &renderer->swapchain.images, &renderer->swapchain.image_views));
    TRY(vulkan_render_pass_create(renderer->ctx.logical_device, &renderer->swapchain.image_format, &renderer->graphics_pipeline.render_pass));
    TRY(vulkan_framebuffers_create(renderer->ctx.logical_device, renderer->graphics_pipeline.render_pass, renderer->swapchain.extent, &renderer->swapchain.image_views, &renderer->swapchain.framebuffers));
    TRY(vulkan_graphics_pipeline_create(renderer->ctx.logical_device, renderer->graphics_pipeline.render_pass, renderer->swapchain.extent, &renderer->graphics_pipeline.layout, &renderer->graphics_pipeline.handle));
    TRY(vulkan_command_pool_create(renderer->ctx.logical_device, &indices, &renderer->command_pool));

    Array(VkSemaphorePtr) semaphores = {0};
    TRY(tda_from(&semaphores, (void*)((VkSemaphore*[]){&renderer->frames.image_available_semaphore, &renderer->frames.render_finished_semaphore}), 2));
    Array(VkFencePtr) fences = {0};
    TRY(tda_from(&fences, (void *)(VkFence*[]){&renderer->frames.in_flight_fence}, 1));
    TRY(vulkan_sync_objects_create(renderer->ctx.logical_device, &semaphores, &fences));

    return RESULT_OK;
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

