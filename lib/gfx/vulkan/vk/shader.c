#include <vulkan/vulkan_core.h>
#include <utils/result_tools.h>
#include <utils/utils.h>

Result
vulkan_shader_module_create(VkDevice device, const char *code, uint32_t code_size, VkShaderModule *module) {
    VkShaderModuleCreateInfo info = {0};

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code_size;
    info.pCode = (uint32_t *)code;

    VK_TRY(vkCreateShaderModule(device, &info, NULL, module));

    return RESULT_OK;
}
