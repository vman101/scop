#include "vk.h"
#include <vulkan/vulkan_core.h>
#include <utils/result_tools.h>
#include <utils/utils.h>

static Result
vulkan_shader_module_create(VkDevice device, Array(char) *file_buffer, VkShaderModule *module) {
    VkShaderModuleCreateInfo info = {0};

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = tda_size(file_buffer);
    info.pCode = (uint32_t *)tda_data(file_buffer);

    VK_TRY(vkCreateShaderModule(device, &info, NULL, module));

    return RESULT_OK;
}

Result
vulkan_shader_module_create_from_file(VkDevice device, const char *filename, VkShaderModule *module) {
    Array(char) da = {0};

    TRY(read_file(filename, "rb", &da));
    TRY(vulkan_shader_module_create(device, &da, module));
    tda_destroy(&da);

    return RESULT_OK;
}
