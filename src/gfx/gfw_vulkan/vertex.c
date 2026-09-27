#include "vk.h"
#include <stddef.h>
#include <vulkan/vulkan_core.h>

VkVertexInputBindingDescription
vulkan_vertex_input_bind_desc_get(void) {
    VkVertexInputBindingDescription bind_desc = {0};

    bind_desc.binding   = 0;
    bind_desc.stride    = sizeof(Vertex);
    bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bind_desc;
}

VertexAttrDescs
vulkan_vertex_input_attr_desc_get(void) {
    VertexAttrDescs descs = {0};

    descs.attrs[0].binding = 0;
    descs.attrs[0].location = 0;
    descs.attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    descs.attrs[0].offset = offsetof(Vertex, pos);
    descs.attrs[1].binding = 0;
    descs.attrs[1].location = 1;
    descs.attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descs.attrs[1].offset = offsetof(Vertex, color);

    return descs;
}
