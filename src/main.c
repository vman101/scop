#include "interface/native_window.h"
#include "interface/gfx.h"
#include "interface/platform.h"
#include <stddef.h>
#include <core/math.h>
#include <stdlib.h>
#include "core/result.h"
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
    GfxDevice dev = {0};
    PlatformWindow window;

    TRY(platform_window_init(800, 600, "TEST", &window));

    CoreNativeWindow native_window = platform_native_window_get(window);

    GfxDeviceDesc dev_desc = {
        .app_name = "name",
        .debug_mode = true,
        .window = &native_window,
    };

    TRY(gfx_device_create(&dev_desc, &dev));
    platform_window_resize_callback_set(window, on_frame_buffer_resize, dev);

    AssetObjData data = asset_obj_data_init();
    AssetParseDebugTracker tracker = {0};
    const char *asset_path = "assets/resources/teapot.obj";

    Result r = asset_obj_file_load(asset_path, &tracker, &data);
    if (r != RESULT_OK) {
        fprintf(stderr, "Error: Failed to parse %s line: %u pos: %u with error %s\n", asset_path, tracker.line, tracker.cursor, result_str(r));
        return EXIT_FAILURE;
    }

    AssetMaterial mat = {0};
    const char *mtl_path = "assets/resources/42.mtl";
    r = asset_mtl_file_load(mtl_path, &tracker, &mat);
    if (r != RESULT_OK) {
        fprintf(stderr, "Error: Failed to parse %s line: %u pos: %u with error %s\n", asset_path, tracker.line, tracker.cursor, result_str(r));
        return EXIT_FAILURE;
    }

    asset_debug_mtl_print(&mat);

    return 0;

    asset_obj_debug_print_index_arr("Indices", &data.indices);

    Array(Vertex) vertices = *(Array(Vertex)*)&data.positions;
    asset_obj_debug_print_vec3_arr("Vertexes", (Array(Vec3) *)&vertices);

    GfxBufferDesc buf_desc = {
        .usage = GFX_BUFFER_VERTEX,
        .mem = GFX_MEMORY_UPLOAD,
        .count = tda_size(&vertices),
        .member_size = sizeof(tda_type(&vertices)),
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
        .member_size = sizeof(tda_type(&idx_data)),
    };

    TRY(gfx_buffer_create(dev, &idx_buf_desc, &idx));

    GfxPipelineDesc pipeline_desc = {
        .depth_test = false,
        .vertex_layout = vertex_layout,
    };
    GfxPipeline graphics_pipeline;

    TRY(gfx_pipeline_create(dev, &pipeline_desc, &graphics_pipeline));

    _Static_assert(sizeof(Vertex) == sizeof(Vec3), "vertex layout mismatch");
    _Static_assert(sizeof(tda_type(&idx_data)) == sizeof(uint32_t), "tda_type broken");
    _Static_assert(sizeof(tda_type(&vertices)) == sizeof(Vertex), "tda_type broken");

    while (!platform_window_should_close_get(window)) {
        platform_event_poll(window);
        if (platform_key_is_pressed(window, PLATFORM_KEY_ESCAPE)) {
            platform_window_should_close_set(window, PLATFORM_TRUE);
        }
        GfxFrame f;
        TRY(gfx_frame_begin(dev, &f));
        gfx_pass_begin(f, (float[4]){ 1, 0, 0, 1.0F });
        gfx_bind_pipeline(f, graphics_pipeline);
        gfx_draw_indexed(f, buf, idx);
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    // gfx_device_destroy(dev);
    return 0;
}
