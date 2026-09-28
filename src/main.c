#include "GLFW/glfw3.h"
#include <core/core.h>
#include "core/native_window.h"
#include "gfx/gfx.h"
#include "platform/platform.h"
#include <core/vertex.h>
#include <stddef.h>

#define ALLOCATOR_IMPLEMENTATION

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

const Vertex vertices[] = {
    // triangle 1
    {{ 0.0F, -0.5F}, {1.0F, 0.0F, 0.0F}},
    {{ 0.5F, -0.5F}, {0.0F, 1.0F, 0.0F}},
    {{-0.5F,  0.5F}, {0.0F, 0.0F, 1.0F}},
    // triangle 2
    {{ 0.5F, -0.5F}, {0.0F, 1.0F, 0.0F}},
    {{ 0.5F,  0.5F}, {1.0F, 1.0F, 0.0F}},
    {{-0.5F,  0.5F}, {0.0F, 0.0F, 1.0F}},
};

static void on_frame_buffer_resize(void *dev, int width, int height) {
    gfx_resize(dev, width, height);
}

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

    GfxBufferDesc buf_desc = {
        .usage = GFX_BUFFER_VERTEX,
        .mem = GFX_MEMORY_UPLOAD,
        .size = sizeof(vertices),
        .data = vertices,
    };

    GfxBuffer buf = {0};
    TRY(gfx_buffer_create(dev, &buf_desc, &buf));

    GfxVertexLayout vertex_layout = {
         .bindings = {
            {
                .stride = sizeof(*vertices),
                .attributes = {
                    { .location = 0, .format = GFX_FORMAT_FLOAT2, .offset = offsetof(Vertex, pos) },
                    { .location = 1, .format = GFX_FORMAT_FLOAT3, .offset = offsetof(Vertex, color) },
                },
                .attribute_count = 2,
            },
        },
        .binding_count = 1,
    };

    GfxPipelineDesc pipeline_desc = {
        .depth_test = false,
        .vertex_layout = vertex_layout,
    };
    GfxPipeline graphics_pipeline;

    TRY(gfx_pipeline_create(dev, &pipeline_desc, &graphics_pipeline));

    while (!platform_window_should_close_get(window)) {
        platform_event_poll(window);
        if (platform_key_is_pressed(window, GLFW_KEY_ESCAPE)) {
            platform_window_should_close_set(window, GLFW_TRUE);
        }
        GfxFrame f;
        TRY(gfx_frame_begin(dev, &f));
        gfx_pass_begin(f, (float[4]){ 1, 0, 1, 1.0F });
        gfx_bind_pipeline(f, graphics_pipeline);
        gfx_draw(f, buf, ARRAY_LEN(vertices));
        gfx_pass_end(f);
        TRY(gfx_frame_end(f));
    }
    // gfx_device_destroy(dev);
    return 0;
}
