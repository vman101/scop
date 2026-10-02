#include <sys/time.h>
#include <time.h>
#include "interface/mage_native_window.h"
#include "interface/mage_gfx.h"
#include "interface/mage_platform.h"
#include <stddef.h>
#include "interface/mage_math.h"
#include <stdlib.h>
#include "interface/mage_result.h"
#include "asset/asset.h"
#include "utils/result_tools.h"

#define ALLOCATOR_IMPLEMENTATION

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

typedef struct {
    Vec3 pos;
} Vertex;

typedef struct {
    GfxBuffer vertices;
    GfxBuffer indixes;
} Mesh;

typedef Vec3 Camera;

static void on_frame_buffer_resize(void *dev, int width, int height) {
    gfx_resize(dev, width, height);
}

Result sdk_asset_obj_indices_get(AssetObjData *obj, Array(uint32_t) *indices) {
    TRY(tda_create(indices, tda_size(&obj->indices)));
    for (size_t i = 0; i < tda_size(&obj->indices); ++i) {
        AssetObjIndex *index = tda_at(&obj->indices, i);
        uint32_t pos = index->position - 1;
        TRY(tda_push(indices, &pos));
    }
    return RESULT_OK;
}

DECLARE_ARRAY(Vertex);

int main(void) {
    GfxDevice dev          = {0};
    PlatformWindow window  = {0};
    const uint32_t  width  = 800;
    const uint32_t  height = 600;

    TRY(platform_window_init(800, 600, "TEST", &window));

    CoreNativeWindow native_window = platform_native_window_get(window);

    GfxDeviceDesc dev_desc = {
        .app_name = "name",
        .debug_mode = true,
        .window = &native_window,
    };

    TRY(gfx_device_create(&dev_desc, &dev));
    platform_window_resize_callback_set(window, on_frame_buffer_resize, dev);

    AssetObjData data = {0};
    AssetParseDebugTracker tracker = {0};
    const char *asset_path = "assets/resources/42.obj";

    Result r = asset_obj_file_load(asset_path, &tracker, &data);
    if (r != RESULT_OK) {
        fprintf(stderr, "Error: Failed to parse %s line: %u pos: %u with error %s\n", asset_path, tracker.line, tracker.cursor, result_str(r));
        return EXIT_FAILURE;
    }

    tracker = (AssetParseDebugTracker){0};
    for (size_t i = 0; i < tda_size(&data.materials); i++) {
        asset_debug_print_mtl(tda_at(&data.materials, i));
    }

    asset_debug_print_index_arr("Indices", &data.indices);

    Array(Vertex) vertices = *(Array(Vertex)*)&data.positions;
    asset_debug_print_vec3_arr("Vertexes", (Array(Vec3) *)&vertices);

    GfxBufferDesc buf_desc = {
        .usage = GFX_BUFFER_VERTEX,
        .mem = GFX_MEMORY_UPLOAD,
        .count = tda_size(&vertices),
        .member_size = tda_sizeof(&vertices),
        .data = tda_data(&vertices),
    };

    GfxBuffer buf = {0};
    TRY(gfx_buffer_create(dev, &buf_desc, &buf));
    GfxVertexLayout vertex_layout = {
         .bindings = {
            {
                .stride = sizeof(Vertex),
                .attributes = {
                    { .location = 0, .format = GFX_FORMAT_FLOAT3, .offset = offsetof(Vertex, pos) },
                },
                .attribute_count = 1,
            },
        },
        .binding_count = 1,
    };

    GfxBuffer idx = {0};
    Array(uint32_t) idx_data = {0};
    TRY(sdk_asset_obj_indices_get(&data, &idx_data));

    GfxBufferDesc idx_buf_desc = {
        .usage = GFX_BUFFER_INDEX,
        .mem = GFX_MEMORY_UPLOAD,
        .data = tda_data(&idx_data),
        .count = tda_size(&idx_data),
        .member_size = tda_sizeof(&idx_data),
    };

    TRY(gfx_buffer_create(dev, &idx_buf_desc, &idx));

    GfxPushConstantDesc pc_mat4_desc = {
        .stage = GFX_SHADER_STAGE_VERTEX,
        .offset = 0,
        .size = sizeof(Mat4),
    };

    GfxPipelineLayoutDesc layout_desc = {
        .push_constant_count = 1,
        .push_constant_descs = (GfxPushConstantDesc[]){ pc_mat4_desc },
    };

    GfxPipelineDesc pipeline_desc = {
        .depth_test = false,
        .vertex_layout = &vertex_layout,
        .vertex_shader_path = "obj/shaders/shader.vert.spv",
        .fragment_shader_path = "obj/shaders/shader.frag.spv",
        .layout = &layout_desc,
    };
    GfxPipeline graphics_pipeline;

    TRY(gfx_pipeline_create(dev, &pipeline_desc, &graphics_pipeline));

    _Static_assert(sizeof(Vertex) == sizeof(Vec3), "vertex layout mismatch");
    _Static_assert(tda_sizeof(&idx_data) == sizeof(uint32_t), "tda_type broken");
    _Static_assert(tda_sizeof(&vertices) == sizeof(Vertex), "tda_type broken");

    float ax = 45.0F;
    float ay = 45.0F;
    float az = 45.0F;

    Mat4 p = mat4_perspective(radians(45.0F), (float)width/(float)height, .1F, 100.0F);

    while (!platform_window_should_close_get(window)) {
        platform_event_poll(window);
        if (platform_key_is_pressed(window, PLATFORM_KEY_ESCAPE)) {
            platform_window_should_close_set(window, PLATFORM_TRUE);
        } else if (platform_key_is_pressed(window, PLATFORM_KEY_W)) {
            ay += 5.F;
        } else if (platform_key_is_pressed(window, PLATFORM_KEY_A)) {
            ax -= 5.F;
        } else if (platform_key_is_pressed(window, PLATFORM_KEY_S)) {
            ay -= 5.F;
        } else if (platform_key_is_pressed(window, PLATFORM_KEY_D)) {
            ax += 5.F;
        }

        Mat4 t = mat4_translate(vec3(0.0F, 0.0F, -8.F));
        Mat4 rx = mat4_rotate_x(radians(ax));
        Mat4 ry = mat4_rotate_y(radians(ay));
        Mat4 rz = mat4_rotate_z(radians(az));
        Mat4 r  = mat4_mul(rz, mat4_mul(ry, rx));
        Mat4 rt = mat4_mul(t, r);
        Mat4 m = mat4_mul(p, rt);
        GfxFrame f;
        TRY(gfx_frame_begin(dev, &f));
        gfx_pass_begin(f, (float[4]){ 1, 0, 0, 1.0F });
        gfx_bind_pipeline(f, graphics_pipeline);
        gfx_push_constant_mat4(f, m);
        gfx_draw_indexed(f, buf, idx);
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    // gfx_device_destroy(dev);
    return 0;
}
