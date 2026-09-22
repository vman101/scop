#include "scop.h"
#include <vulkan/vulkan_core.h>

Result
vulkan_shader_module_create(VkDevice device, DynamicArray *file_buffer, VkShaderModule module) {
    VkShaderModuleCreateInfo info = {0};

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = file_buffer->size;
    info.pCode = file_buffer->data;

    VK_TRY(vkCreateShaderModule(device, &info, nullptr, &module));

    return RESULT_OK;
}

Result
vulkan_shader_module_create_from_file(VkDevice device, const char *filename, VkShaderModule module) {
    DynamicArray da = {0};

    TRY(read_file(filename, &da));
    TRY(vulkan_shader_module_create(device, &da, module));

    return RESULT_OK;
}
