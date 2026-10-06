#include "scop.h"
#include <stdlib.h>
#include <utils/result_tools.h>
#include "mesh/mesh.h"

void on_frame_buffer_resize(void *dev, int width, int height) {
    gfx_resize(dev, width, height);
}


Result scop_create(Scop *scop) {
    GfxDevice *dev = &scop->dev;
    PlatformWindow window = scop->window;
    GfxPipeline *pipeline = &scop->pipeline;
    Result r = RESULT_OK;

    CoreNativeWindow native_window = platform_native_window_get(window);
    int32_t width;
    int32_t height;

    platform_framebuffer_size_get(window, &width, &height);

    GfxDeviceDesc dev_desc = {
        .app_name   = "name",
        .debug_mode = true,
        .window     = &native_window,
        .width      = width,
        .height     = height,
    };

    TRY_GOTO(r, cleanup, gfx_device_create(&dev_desc, dev));
    platform_window_resize_callback_set(window, on_frame_buffer_resize, *dev);

    GfxVertexLayout vertex_layout = {
         .bindings = {
            {
                .stride = sizeof(Vertex),
                .attributes = {
                    { .location = 0, .format = GFX_FORMAT_FLOAT3, .offset = offsetof(Vertex, pos)     },
                    { .location = 1, .format = GFX_FORMAT_FLOAT3, .offset = offsetof(Vertex, normal)  },
                    { .location = 2, .format = GFX_FORMAT_FLOAT2, .offset = offsetof(Vertex, uv)      },
                    { .location = 3, .format = GFX_FORMAT_UINT,  .offset = offsetof(Vertex, face_id) },
                },
                .attribute_count = 4,
            },
        },
        .binding_count = 1,
    };

    GfxPushConstantDesc pc_descs[] = {
        { .stage = GFX_SHADER_STAGE_VERTEX,   .offset = 0,                .size = sizeof(Mat4) * 2 },
        { .stage = GFX_SHADER_STAGE_FRAGMENT, .offset = sizeof(Mat4) * 2, .size = sizeof(uint32_t) + sizeof(float) },
    };

    scop->pc_vert = pc_descs[0];
    scop->pc_frag = (GfxPushConstantDesc){ .stage = GFX_SHADER_STAGE_FRAGMENT, .offset = sizeof(Mat4) * 2, .size = sizeof(uint32_t) };
    scop->pc_frag2 = (GfxPushConstantDesc){ .stage = GFX_SHADER_STAGE_FRAGMENT, .offset = ((sizeof(Mat4) * 2) + sizeof(uint32_t)), .size = sizeof(float) };
;

    GfxPipelineLayoutDesc layout_desc = {
        .push_constant_count = ARRAY_LEN(pc_descs),
        .push_constant_descs = pc_descs,
    };

    Array(uint8_t) buf = {0};
    TRY_GOTO(r, cleanup, read_file("obj/shaders/shader.vert.spv", "rb", &buf));
    GfxShaderDesc vert_shader = {
        .code  = tda_data(&buf),
        .size  = tda_size(&buf),
        .stage = GFX_SHADER_STAGE_VERTEX,
    };

    TRY_GOTO(r, cleanup, read_file("obj/shaders/shader.frag.spv", "rb", &buf));
    GfxShaderDesc frag_shader = {
        .code  = tda_data(&buf),
        .size  = tda_size(&buf),
        .stage = GFX_SHADER_STAGE_FRAGMENT,
    };

    GfxShaderDesc shaders[] = (GfxShaderDesc[]){ vert_shader, frag_shader };
    GfxPipelineDesc pipeline_desc = {
        .depth_test    = false,
        .layout        = &layout_desc,
        .shaders       = shaders,
        .shader_count  = ARRAY_LEN(shaders),
        .vertex_layout = &vertex_layout,
    };

    TRY_GOTO(r, cleanup, gfx_pipeline_create(*dev, &pipeline_desc, pipeline));

    r = RESULT_OK;
cleanup:
    free((void *)vert_shader.code);
    free((void *)frag_shader.code);

    return r;
}

void scop_destroy(Scop *scop) {
    gfx_pipeline_destroy(scop->dev, scop->pipeline);
    gfx_device_destroy(scop->dev);
}

