#include "gfx_vulkan_internal.h"
#include "interface/mage_gfx.h"
#include "utils/da.h"
#include <utils/result_tools.h>
#include <utils/utils.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static VkShaderStageFlags
gfx_shader_stage_to_vk(GfxShaderStage s) {
    switch (s) {
        case GFX_SHADER_STAGE_VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
        case GFX_SHADER_STAGE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

// gfx_vulkan.c
static VkFormat gfx_format_to_vk(GfxFormat f) {
    switch (f) {
        case GFX_FORMAT_FLOAT:       return VK_FORMAT_R32_SFLOAT;
        case GFX_FORMAT_FLOAT2:      return VK_FORMAT_R32G32_SFLOAT;
        case GFX_FORMAT_FLOAT3:      return VK_FORMAT_R32G32B32_SFLOAT;
        case GFX_FORMAT_FLOAT4:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GFX_FORMAT_UBYTE4_NORM: return VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_FORMAT_UNDEFINED;
}

Result
gfx_pipeline_create(GfxDevice_T *dev, GfxPipelineDesc *desc, GfxPipeline *out) {
    GfxPipeline pipeline = alloc(sizeof(*pipeline));

    Array(VkPipelineShaderStageCreateInfo) shader_stages = {0};
    if (desc->shader_count > 0) {
        for (size_t i = 0; i < desc->shader_count; i++) {
            GfxShaderDesc *s_desc = &desc->shaders[i];
            VkShaderModule shader = {0};
            TRY(vulkan_shader_module_create(dev->ctx.logical_device, s_desc->code, s_desc->size, &shader));

            VkPipelineShaderStageCreateInfo stage_info;
            memset(&stage_info, 0, sizeof(stage_info));
            stage_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage_info.stage               = gfx_shader_stage_to_vk(s_desc->stage);
            stage_info.module              = shader;
            stage_info.pName               = "main";
            stage_info.pSpecializationInfo = NULL;

            tda_push(&shader_stages, &stage_info);
        }
    }

    VkVertexInputBindingDescription bind_desc[GFX_MAX_BINDINGS] = {0};
    VkVertexInputAttributeDescription attr_desc[GFX_MAX_BINDINGS * GFX_MAX_ATTRIBUTES] = {0};
    uint32_t attr_count = 0;

    for (size_t i = 0; i < desc->vertex_layout->binding_count; i++) {
        GfxVertexBinding *binding = &desc->vertex_layout->bindings[i];
        bind_desc[i] = (VkVertexInputBindingDescription) {
            .binding = i,
            .stride = binding->stride,
            .inputRate = binding->per_instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX,
        };
        for (size_t j = 0; j < binding->attribute_count; j++) {
            GfxVertexAttribute *attr = &binding->attributes[j];
            attr_desc[attr_count++] = (VkVertexInputAttributeDescription) {
                .binding = i,
                .format = gfx_format_to_vk(attr->format),
                .offset = attr->offset,
                .location = attr->location,
            };
        }
    }

    VkPipelineVertexInputStateCreateInfo vert_input_info = {0};
    vert_input_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vert_input_info.vertexBindingDescriptionCount        = desc->vertex_layout->binding_count;
    vert_input_info.pVertexBindingDescriptions           = bind_desc;
    vert_input_info.vertexAttributeDescriptionCount      = attr_count;
    vert_input_info.pVertexAttributeDescriptions         = attr_desc;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ARRAY_LEN(dynamic_states),
        .pDynamicStates = dynamic_states,
    };
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,   // pointers stay NULL, values come at record time
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_LINE;
    rasterizer.lineWidth               = 1.0F;
    rasterizer.cullMode                = VK_CULL_MODE_NONE;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0F;
    rasterizer.depthBiasClamp          = 0.0F;
    rasterizer.depthBiasSlopeFactor    = 0.0F;

    VkPipelineMultisampleStateCreateInfo multisampling;
    memset(&multisampling, 0, sizeof(multisampling));
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0F;
    multisampling.pSampleMask           = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;
    multisampling.pNext                 = NULL;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask      =
        VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend = {0};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_COPY;
    color_blend.attachmentCount = 1;
    color_blend.pAttachments = &color_blend_attachment;
    color_blend.blendConstants[0] = 0.0F;
    color_blend.blendConstants[1] = 0.0F;
    color_blend.blendConstants[2] = 0.0F;
    color_blend.blendConstants[3] = 0.0F;

    GfxPushConstantDesc *pc_descs = desc->layout->push_constant_descs;

    VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = pc_descs->size,
        .offset = pc_descs->offset,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    VkPipelineLayout pipeline_layout;
    pipeline_layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount             = 0;
    pipeline_layout_info.pSetLayouts                = NULL;
    pipeline_layout_info.pushConstantRangeCount     = desc->layout->push_constant_count;
    pipeline_layout_info.pPushConstantRanges        = &push_range;
    VK_TRY(vkCreatePipelineLayout(dev->ctx.logical_device, &pipeline_layout_info, NULL, &pipeline_layout));
    pipeline->layout = pipeline_layout;

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount                   = tda_size(&shader_stages);
    pipeline_info.pStages                      = tda_data(&shader_stages);
    pipeline_info.pVertexInputState            = &vert_input_info;
    pipeline_info.pInputAssemblyState          = &input_assembly;
    pipeline_info.pViewportState               = &viewport_state;
    pipeline_info.pRasterizationState          = &rasterizer;
    pipeline_info.pMultisampleState            = &multisampling;
    pipeline_info.pDepthStencilState           = NULL;
    pipeline_info.pColorBlendState             = &color_blend;
    pipeline_info.pDynamicState                = &dynamic;
    pipeline_info.layout                       = pipeline_layout;
    pipeline_info.renderPass                   = dev->render_pass;
    pipeline_info.subpass                      = 0;
    pipeline_info.basePipelineHandle           = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex            = -1;
    VK_TRY(vkCreateGraphicsPipelines(dev->ctx.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline->handle));
    *out = pipeline;
    dev->graphics_pipeline = pipeline;

    for (size_t i = 0; i < tda_size(&shader_stages); i++) {
        VkPipelineShaderStageCreateInfo *stage = tda_get(&shader_stages, i);
        if (stage) {
            vkDestroyShaderModule(dev->ctx.logical_device, stage->module, NULL);
        }
    }
    return RESULT_OK;
}
