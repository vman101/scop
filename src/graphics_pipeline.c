#include "scop.h"
#include <vulkan/vulkan_core.h>

Result
vulkan_graphics_pipeline_create(VkDevice device, VkExtent2D *extend) {
    VkShaderModule frag_shader = {0};
    VkShaderModule vert_shader = {0};

    TRY(vulkan_shader_module_create_from_file(device, "obj/vert.spv", vert_shader));
    TRY(vulkan_shader_module_create_from_file(device, "obj/frag.spv", frag_shader));

    VkPipelineShaderStageCreateInfo vert_stage_info;
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_shader;
    vert_stage_info.pName = "main";
    vert_stage_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo frag_stage_info;
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_shader;
    frag_stage_info.pName = "main";
    frag_stage_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vert_stage_info, frag_stage_info };
    VkPipelineVertexInputStateCreateInfo vert_input_info = {0};
    vert_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vert_input_info.vertexBindingDescriptionCount = 0;
    vert_input_info.pVertexBindingDescriptions = nullptr;
    vert_input_info.vertexAttributeDescriptionCount = 0;
    vert_input_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport =  {0};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = (float)extend->width;
    viewport.height = (float)extend->height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = *extend;

    vkDestroyShaderModule(device, vert_shader, nullptr);
    vkDestroyShaderModule(device, frag_shader, nullptr);
    return RESULT_OK;
}
