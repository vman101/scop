#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "asset/asset.h"
#include "interface/mage_native_window.h"
#include "interface/mage_gfx.h"
#include "interface/mage_platform.h"
#include <stddef.h>
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "mesh/mesh.h"
#include "utils/da.h"
#include "utils/result_tools.h"
#include "utils/sv.h"
#include "utils/utils.h"

#define ALLOCATOR_IMPLEMENTATION

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

static void on_frame_buffer_resize(void *dev, int width, int height) {
    gfx_resize(dev, width, height);
}

typedef struct {
    GfxDevice           dev;
    PlatformWindow      window;
    GfxPipeline         pipeline;
    GfxPushConstantDesc pc_vert;
    GfxPushConstantDesc pc_frag;
    GfxPushConstantDesc pc_frag2;
    GfxTexture          tex;
} Scop;

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

#define FPS 60
#define FRAME_TIME (1 / FPS)

void print_help(void) {
    fprintf(stderr,
            "Usage: scop <path-to-obj> <path-to-tex>\n"
    );
}

int main(int argc, const char *argv[]) { // NOLINT(readability-function-cognitive-complexity)
    Result          r      = RESULT_OK;
    Scop            scop   = {0};
    const  uint32_t width  = 800;
    const  uint32_t height = 600;

    if (argc < 2) {
        fprintf(stderr, "Invalid args: expect 1 + 1 optional\n");
        print_help();
        return EXIT_FAILURE;
    }

    TRY_GOTO(r, cleanup, platform_window_create(800, 600, "TEST", &scop.window));
    TRY_GOTO(r, cleanup, scop_create(&scop));

    Mesh                   mesh     = {0};
    AssetObjData           obj_data = {0};
    AssetParseDebugTracker tracker  = {0};
    AssetImage             i        = {0};

    const char *obj_filename        = argv[1];
    const char *tex_path            = argc == 3 ? argv[2] : "assets/kittens.ppm";
    StringView s                    = { obj_filename, strlen(obj_filename) };
    StringView path                 = sv_chop_last(&s, '/');

    Array(uint8_t) content = {0};
    TRY_GOTO(r, cleanup, read_file(tex_path, "r", &content));
    TRY_GOTO(r, cleanup, asset_image_parse_ppm(&content, &i));
    GfxTextureDesc tex_desc = {
        .data   = i.px,
        .width  = i.w,
        .height = i.h
    };
    TRY_GOTO(r, cleanup, gfx_texture_create(scop.dev, &tex_desc, &scop.tex));

    tda_destroy(&content);
    free(i.px);
    TRY_GOTO(r, cleanup, read_file(obj_filename, "r", &content));
    r = asset_obj_file_parse((Array(char) *)&content, &tracker, &obj_data);
    if (r != RESULT_OK) {
        fprintf(stderr, "Error while parsing file %s at %u:%u : %s\n", obj_filename, tracker.line, tracker.cursor, result_str(r));
        goto cleanup;
    }

    Array(AssetMtlLib) mtl_libs = {0};
    for (size_t i = 0; i < tda_size(&obj_data.mtl_lib); i++) {
        tda_destroy(&content);
        AssetObjMtlLib *l = tda_at(&obj_data.mtl_lib, i);
        if (l) {
            Result t;
            char file_path[PATH_MAX] = {0};
            sv_strcopy(path, file_path);
            strncat(file_path, "/", PATH_MAX);
            strncat(file_path, l->name, PATH_MAX);
            AssetMtlLib mtl_lib = {0};
            t = read_file(file_path, "r", &content);
            if (t != RESULT_OK) {
                fprintf(stderr, "Error while reading file %s: %s\n", file_path, result_str(t));
                continue ;
            }
            tracker = (AssetParseDebugTracker){0};
            asset_mtl_file_parse((Array(char) *)&content, &tracker, &mtl_lib.mtls);
            if (t != RESULT_OK) {
                fprintf(stderr, "Error while parsing file %s at %u:%u : %s\n", obj_filename, tracker.line, tracker.cursor, result_str(t));
            }
            TRY_GOTO(r, cleanup, tda_push(&mtl_libs, &mtl_lib));
        }
    }

    TRY(mesh_create(scop.dev, &obj_data, &mtl_libs, &mesh));
    for (size_t i = 0; i < tda_size(&mtl_libs); i++) {
        AssetMtlLib *l = tda_get(&mtl_libs, i);
        asset_mtl_destroy(l);
    }
    asset_obj_destroy(&obj_data);

    float ax = 0.0F;
    float ay = 0.0F;
    float az = 0.0F;

    int32_t x = 0;
    int32_t y = 0;

    Vec3 eye = vec3(0, 0, 8);
    Mat4 c = mat4_translate(vec3_negate(mesh_center_get(&mesh)));
    uint64_t last = platform_time_ns();
    Mat4 v = mat4_look_at(eye, mesh.position, vec3(0, 1, 0));


    Vec3 *entities[] = {&mesh.position};
    uint32_t entity_index = 0;


    float tex_mix = 0.0F;
    float tex_target = 0.0F;

    while (!platform_window_should_close_get(scop.window)) {
        platform_event_poll(scop.window);
        platform_window_mouse_pos_get(scop.window, &x, &y);

        Vec3 *focused_pos = entities[entity_index];

        uint64_t now   = platform_time_ns();
        float    dt    = (float)((double)(now - last) / 1e9);
        float    speed = 120.0F;

        last = now;

        ay += 30.0F * dt;
        float translate = 20.F * dt;

        if (platform_key_is_pressed(scop.window, PLATFORM_KEY_ESCAPE)) {
            platform_window_should_close_set(scop.window, PLATFORM_TRUE);
        }
        if (focused_pos) {
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_W))     { ay += dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_S))     { ay -= dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_D))     { ax += dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_A))     { ax -= dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_Q))     { az -= dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_E))     { az += dt * speed; }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_LEFT_SHIFT) || platform_key_is_pressed(scop.window, PLATFORM_KEY_RIGHT_SHIFT)) {
                if (platform_key_is_pressed(scop.window, PLATFORM_KEY_UP))   { *focused_pos = vec3_add(*focused_pos, vec3(0, translate, 0)); }
                if (platform_key_is_pressed(scop.window, PLATFORM_KEY_DOWN)) { *focused_pos = vec3_add(*focused_pos, vec3(0, -translate, 0)); }
            } else                                                           {
                if (platform_key_is_pressed(scop.window, PLATFORM_KEY_UP))   { *focused_pos = vec3_add(*focused_pos, vec3(0, 0, -translate)); }
                if (platform_key_is_pressed(scop.window, PLATFORM_KEY_DOWN)) { *focused_pos = vec3_add(*focused_pos, vec3(0, 0, translate)); }
            }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_LEFT))       { *focused_pos = vec3_add(*focused_pos, vec3(-translate, 0, 0)); }
            if (platform_key_is_pressed(scop.window, PLATFORM_KEY_RIGHT))      { *focused_pos = vec3_add(*focused_pos, vec3(translate, 0, 0)); }
            if (platform_key_was_pressed(scop.window, PLATFORM_KEY_R))         { v = mat4_look_at(eye, *focused_pos, vec3(0, 1, 0)); }
            if (platform_key_was_pressed(scop.window, PLATFORM_KEY_TAB))       { entity_index = (entity_index + 1) % ARRAY_LEN(entities); }
            if (platform_key_was_pressed(scop.window, PLATFORM_KEY_T))         { tex_target = 1.0F - tex_target; }
            if (platform_key_was_pressed(scop.window, PLATFORM_KEY_BACKSPACE)) { ax = ay = az = 0.0F; *focused_pos = (Vec3){0}; }
         }
        float step = dt * 2.0F;
        tex_mix += fmaxf(-step, fminf(step, tex_target - tex_mix));
        Mat4 r    = mat4_mul(
            mat4_rotate_y(radians(ay)),
            mat4_mul(
                mat4_rotate_x(radians(ax)),
                mat4_rotate_z(radians(az))
            )
        );
        Mat4 t    = mat4_translate(*focused_pos);
        Mat4 m    = mat4_mul(t, mat4_mul(r, c));
        Mat4 proj = mat4_perspective(radians(45.0F), (float)width / (float)height, 0.1F, 100.0F);
        Mat4 mvp  = mat4_mul(proj, mat4_mul(v, m));

        GfxFrame f;
        TRY(gfx_frame_begin(scop.dev, &f));
        if (!f) continue;

        struct { Mat4 mvp; Mat4 model; } push = { .mvp = mvp, .model = m };

        gfx_pass_begin(f, (float[4]){ (1.F / 0x20), 1.F / 0x20, 1.F / 0x20, 1 });
        gfx_bind_pipeline(f, scop.pipeline);
        gfx_push_constant(f, &scop.pc_vert, &push);
        gfx_push_constant(f, &scop.pc_frag2, &tex_mix);
        mesh_draw(f, &mesh, &scop.pc_frag);
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    r = RESULT_OK;

cleanup:
    tda_destroy(&content);
    tda_destroy(&mtl_libs);
    mesh_destroy(scop.dev, &mesh);
    gfx_texture_destroy(scop.dev, scop.tex);
    platform_window_destroy(scop.window);
    scop_destroy(&scop);
    result_str(r);
    return r;
}
