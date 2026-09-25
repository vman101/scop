#include "renderer.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>

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
    vkGetPhysicalDeviceProperties(renderer->device, &p);       // p.apiVersion is the device version

    printf("GPU vulkan version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(p.apiVersion),
        VK_API_VERSION_MINOR(p.apiVersion),
        VK_API_VERSION_PATCH(p.apiVersion)
    );
}

Result
renderer_vulkan_init(Renderer *renderer, const char **validation_layers, const uint32_t layers_count) {
    VkInstanceCreateInfo create_info = {0};
    VkApplicationInfo renderer_info       = {0};

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

    VK_TRY(vkCreateInstance(&create_info, nullptr, &renderer->vk));

    if (debug_mode) {
        TRY(vulkan_debug_messenger_create(renderer->vk, &debug_mes_info, &renderer->mes));
    }

    da_destroy(&da);
    TRY(renderer_create_surface(renderer));
    TRY(vulkan_device_pick(renderer->vk, renderer->surface, &renderer->device));
    QueueFamilyIndices indices;
    TRY(vulkan_logical_device_create(renderer->device, renderer->surface, &renderer->log_dev, &indices));

    renderer_physical_device_properties_log(renderer);

    vkGetDeviceQueue(renderer->log_dev, indices.graphics_family, 0, &renderer->graphics_queue);
    vkGetDeviceQueue(renderer->log_dev, indices.present_family, 0, &renderer->present_queue);

    VkSwapchainCreateInfoKHR swapchain_info;
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(renderer->window, &width, &height);
    TRY(vulkan_swapchain_info_create(
        renderer->device,
        renderer->surface,
        width,
        height,
        &swapchain_info,
        &renderer->swapchain_image_format,
        &renderer->swapchain_extent
    ));
    VK_TRY(vkCreateSwapchainKHR(renderer->log_dev, &swapchain_info, nullptr, &renderer->swapchain));

    uint32_t image_count = 0;
    VK_TRY(vkGetSwapchainImagesKHR(renderer->log_dev, renderer->swapchain, &image_count, nullptr));
    TRY(tda_create(&renderer->swapchain_images, image_count));
    VK_TRY(vkGetSwapchainImagesKHR(
        renderer->log_dev,
        renderer->swapchain,
        &image_count,
        tda_data(&renderer->swapchain_images)
    ));
    tda_size(&renderer->swapchain_images) = image_count;

    TRY(vulkan_swapchain_image_views_create_from_image(renderer->log_dev, renderer->swapchain_image_format, &renderer->swapchain_images, &renderer->swapchain_image_views));
    TRY(vulkan_render_pass_create(renderer->log_dev, &renderer->swapchain_image_format, &renderer->render_pass));
    TRY(vulkan_framebuffers_create(renderer->log_dev, &renderer->framebuffers, &renderer->swapchain_image_views, renderer->render_pass, renderer->swapchain_extent));
    TRY(vulkan_graphics_pipeline_create(renderer->log_dev, renderer->render_pass, &renderer->graphics_pipeline, &renderer->graphics_pipeline_layout, &renderer->swapchain_extent));
    TRY(vulkan_command_pool_create(renderer->log_dev, &renderer->command_pool, &indices));
    TRY(vulkan_command_buffer_create(renderer->log_dev, renderer->command_pool, &renderer->command_buffer));

    Array(VkSemaphorePtr) semaphores = {0};
    TRY(tda_from(&semaphores, (void*)((VkSemaphore*[]){&renderer->image_available_semaphore, &renderer->render_finished_semaphore}), 2));
    Array(VkFencePtr) fences = {0};
    TRY(tda_from(&fences, (void *)(VkFence*[]){&renderer->in_flight_fence}, 1));
    TRY(vulkan_sync_objects_create(renderer->log_dev, &semaphores, &fences));

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

void renderer_destroy(Renderer *renderer) {
    vkDeviceWaitIdle(renderer->log_dev);
    vulkan_debug_messenger_destroy(renderer->vk, renderer->mes);
    vkDestroySemaphore(renderer->log_dev, renderer->image_available_semaphore, nullptr);
    vkDestroySemaphore(renderer->log_dev, renderer->render_finished_semaphore, nullptr);
    vkDestroyFence(renderer->log_dev, renderer->in_flight_fence, nullptr);
    vkDestroyCommandPool(renderer->log_dev, renderer->command_pool, nullptr);
    for (size_t i = 0; i < tda_size(&renderer->buffers); i++) {
        vkDestroyBuffer(renderer->log_dev, *tda_at(&renderer->buffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&renderer->device_memory); i++) {
        vkFreeMemory(renderer->log_dev, *tda_at(&renderer->device_memory, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&renderer->framebuffers); i++) {
        vkDestroyFramebuffer(renderer->log_dev, *tda_at(&renderer->framebuffers, i), nullptr);
    }
    for (size_t i = 0; i < tda_size(&renderer->swapchain_image_views); i++) {
        vkDestroyImageView(renderer->log_dev, *tda_at(&renderer->swapchain_image_views, i), nullptr);
    }
    tda_destroy(&renderer->buffers);
    tda_destroy(&renderer->framebuffers)
    tda_destroy(&renderer->swapchain_image_views);
    tda_destroy(&renderer->swapchain_images);
    vkDestroyPipeline(renderer->log_dev, renderer->graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(renderer->log_dev, renderer->graphics_pipeline_layout, nullptr);
    vkDestroyRenderPass(renderer->log_dev, renderer->render_pass, nullptr);
    vkDestroySwapchainKHR(renderer->log_dev, renderer->swapchain, nullptr);
    vkDestroyDevice(renderer->log_dev, nullptr);
    vkDestroySurfaceKHR(renderer->vk, renderer->surface, nullptr);
    vkDestroyInstance(renderer->vk, nullptr);
    glfwDestroyWindow(renderer->window);
    glfwTerminate();
}
