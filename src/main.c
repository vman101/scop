#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include "asset/asset.h"
#include "interface/mage_gfx.h"
#include "interface/mage_platform.h"
#include <stddef.h>
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "model/model.h"
#include "utils/da.h"
#include "utils/result_tools.h"
#include "utils/utils.h"
#include "scop.h"
#include "loader/loader.h"
#include "geometry/geometry.h"

#define ALLOCATOR_IMPLEMENTATION
bool keys_prev[PLATFORM_KEY_COUNT] = {0};
bool keys_now[PLATFORM_KEY_COUNT]  = {0};

bool scop_key_was_pressed(PlatformWindow window, int key) {
    bool pressed = platform_key_is_pressed(window, key);
    return pressed && keys_prev[key];
}

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

#define FPS 60
#define FRAME_TIME (1 / FPS)

void print_help(void) {
    fprintf(stderr,
            "Usage: scop <path-to-obj> <path-to-tex>\n"
    );
}

Result scop_load_texture(Scop *scop, const char *tex_path, GfxTexture *out) {
    Result r = RESULT_OK;
    AssetImage             i        = {0};
    GfxTexture             tex      = {0};


    Array(uint8_t) content = {0};
    TRY_GOTO(r, cleanup, read_file(tex_path, "r", &content));
    TRY_GOTO(r, cleanup, asset_image_parse_ppm(&content, &i));
    GfxTextureDesc tex_desc = {
        .data   = i.px,
        .width  = i.w,
        .height = i.h
    };
    TRY_GOTO(r, cleanup, gfx_texture_create(scop->dev, &tex_desc, &scop->tex));

    MOVE(out, tex);
    r = RESULT_OK;
cleanup:
    free(i.px);
    tda_destroy(&content);
    return r;
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

    Model model = {0};

    const char *obj_filename    = argv[1];
    const char *tex_path        = argc == 3 ? argv[2] : "assets/kittens.ppm";
    // const char *human_path  = "assets/FinalBaseMesh.obj";

    GfxTexture tex = {0};

    TRY_GOTO(r, cleanup, loader_model_load(scop.dev, obj_filename, &model));
    TRY_GOTO(r, cleanup, scop_load_texture(&scop, tex_path, &tex));

    float ax = 0.0F;
    float ay = 0.0F;
    float az = 0.0F;

    int32_t x = 0;
    int32_t y = 0;

    Vec3 eye = vec3(0, 0, 8);
    Mat4 c = mat4_translate(vec3_negate(geometry_box_center_get(model.mesh.bounding_box)));
    uint64_t last = platform_time_ns();

    Mat4 v = mat4_look_at(eye, model.position, vec3(0, 1, 0));

    Vec3 *entities[] = {&model.position};
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
            if (scop_key_was_pressed(scop.window, PLATFORM_KEY_R))         { v = mat4_look_at(eye, *focused_pos, vec3(0, 1, 0)); }
            // if (scop_key_was_pressed(scop.window, PLATFORM_KEY_TAB))       { entity_index = (entity_index + 1) % ARRAY_LEN(entities); }
            if (scop_key_was_pressed(scop.window, PLATFORM_KEY_T))         { tex_target = 1.0F - tex_target; }
            if (scop_key_was_pressed(scop.window, PLATFORM_KEY_BACKSPACE)) { ax = ay = az = 0.0F; *focused_pos = (Vec3){0}; }
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
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    r = RESULT_OK;

cleanup:
    gfx_texture_destroy(scop.dev, scop.tex);
    model_destroy(scop.dev, &model);
    platform_window_destroy(scop.window);
    scop_destroy(&scop);
    result_str(r);
    return r;
}
