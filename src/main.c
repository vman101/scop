#include <sys/time.h>
#include "interface/mage_native_window.h"
#include "interface/mage_gfx.h"
#include "interface/mage_platform.h"
#include <stddef.h>
#include "interface/mage_math.h"
#include "mesh/mesh.h"
#include "utils/result_tools.h"
#include "utils/utils.h"

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

typedef Vec3 Camera;

static void on_frame_buffer_resize(void *dev, int width, int height) {
    gfx_resize(dev, width, height);
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
    Mesh mesh = {0};
    TRY(mesh_load_from_obj(dev, "assets/resources/teapot2.obj", &mesh));

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

    GfxPushConstantDesc pc_mat4_desc = {
        .stage = GFX_SHADER_STAGE_VERTEX,
        .offset = 0,
        .size = sizeof(Mat4),
    };

    GfxPipelineLayoutDesc layout_desc = {
        .push_constant_count = 1,
        .push_constant_descs = (GfxPushConstantDesc[]){ pc_mat4_desc },
    };

    Array(char) buf = {0};
    TRY(read_file("obj/shaders/shader.vert.spv", "rb", &buf));
    GfxShaderDesc vert_shader = {
        .code  = tda_data(&buf),
        .size  = tda_size(&buf),
        .stage = GFX_SHADER_STAGE_VERTEX,
    };

    TRY(read_file("obj/shaders/shader.frag.spv", "rb", &buf));
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
    GfxPipeline graphics_pipeline;

    TRY(gfx_pipeline_create(dev, &pipeline_desc, &graphics_pipeline));

    float ax = 45.0F;
    float ay = 45.0F;
    float az = 45.0F;

    Mat4 p = mat4_perspective(radians(45.0F), (float)width/(float)height, .1F, 100.0F);

    while (!platform_window_should_close_get(window)) {
        platform_event_poll(window);
        if (platform_key_is_pressed(window, PLATFORM_KEY_ESCAPE)) {
            platform_window_should_close_set(window, PLATFORM_TRUE);
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_W)) {
            ay += 5.F;
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_S)) {
            ay -= 5.F;
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_D)) {
            ax += 5.F;
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_A)) {
            ax -= 5.F;
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_Q)) {
            az -= 5.F;
        }
        if (platform_key_is_pressed(window, PLATFORM_KEY_E)) {
            az += 5.F;
        }

        Mat4 c  = mat4_translate(vec3_negate(mesh_center_get(&mesh)));
        Mat4 rx = mat4_rotate_x(radians(ax));
        Mat4 ry = mat4_rotate_y(radians(ay));
        Mat4 rz = mat4_rotate_z(radians(az));
        Mat4 r  = mat4_mul(ry, mat4_mul(rx, rz));
        Mat4 t  = mat4_translate(vec3(0.0F, 0.0F, -8.0F));
        Mat4 m = mat4_mul(t, mat4_mul(r, c));
        Mat4 mvp  = mat4_mul(p, m);

        GfxFrame f;
        TRY(gfx_frame_begin(dev, &f));
        gfx_pass_begin(f, (float[4]){ 1, 0, 0, 1.0F });
        gfx_bind_pipeline(f, graphics_pipeline);
        gfx_push_constant_mat4(f, mvp);
        mesh_draw(f, &mesh);
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    // gfx_device_destroy(dev);
    return 0;
}
