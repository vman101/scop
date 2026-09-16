#include "scop.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    (void)pUserData;
    (void)messageType;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "validation error: %s\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void
vulkan_debug_messenger_populate(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->pfnUserCallback = debugCallback;
    create_info->pUserData = nullptr;

}

Result
vulkan_debug_messenger_create(
    VkInstance instance,
    VkDebugUtilsMessengerCreateInfoEXT *create_info,
    VkDebugUtilsMessengerEXT *messenger
) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT"
        );
    if (func) {
        VK_TRY(func(instance, create_info, nullptr, messenger));
    } else {
        return RESULT_ERR_VULKAN;
    }

    return RESULT_OK;
}

void
vulkan_debug_messenger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT mes) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT"
        );

    if (func) {
        func(instance, mes, nullptr);
    }
}

void
vulkan_instance_destroy(VkInstance instance) {
    vkDestroyInstance(instance, nullptr);
}

Result
vulkan_validation_layers_check(const char *validation_layers[], const uint32_t layer_count) {
    uint32_t available_count = 0;
    VkLayerProperties *layer_properties = nullptr;

    VK_TRY(vkEnumerateInstanceLayerProperties(&available_count, nullptr));
    layer_properties = alloc(sizeof(*layer_properties) * available_count);
    if (!layer_properties) { return RESULT_ERR_ALLOC; }

    VK_TRY(vkEnumerateInstanceLayerProperties(&available_count, layer_properties));

    Result res = RESULT_OK;

    if (available_count < layer_count) {
        fprintf(stderr, "Not enough Vulkan validation layers available: requested %u, got %u", layer_count, available_count);
        res = RESULT_ERR_VULKAN;
        goto out;
    }

    for (uint32_t i = 0; i < layer_count && res == RESULT_OK; i++) {
        bool layer_found = false;
        for (uint32_t j = 0; j < available_count; j++) {
            if (strcmp(validation_layers[i], layer_properties[j].layerName) == 0) {
                layer_found = true;
                break ;
            }
        }
        if (!layer_found) {
            fprintf(stderr, "Vulkan validation layer %s is not available!\n", validation_layers[i]);
            res = RESULT_ERR_VULKAN;
            goto out;
        }
    }

out:
    free(layer_properties);

    return res;
}
