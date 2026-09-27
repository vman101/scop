#pragma once
#include <stdint.h>
#include <core/result.h>

typedef struct GfxDevice    GfxDevice;
typedef void                GfxWindow;

typedef struct GfxBuffer    GfxBuffer;
typedef struct GfxPipeline  GfxPipeline;
typedef struct GfxFrame     GfxFrame;
typedef struct GfxVertexLayout GfxVertexLayout;

typedef enum { GFX_BUFFER_VERTEX, GFX_BUFFER_INDEX, GFX_BUFFER_UNIFORM } GfxBufferUsage;
typedef enum { GFX_MEMORY_GPU, GFX_MEMORY_UPLOAD, GFX_MEMORY_READBACK } GfxMemoryKind;

typedef struct {
    const char *vertex_shader;
    const char *fragment_shader;
    GfxVertexLayout *layout;
    bool depth_test;
} GfxPipelineDesc;

Result gfx_device_create(GfxDevice *dev, GfxWindow *window, const char **validation_layers, uint32_t layers_count);
Result gfx_buffer_create(GfxDevice *dev, GfxBufferUsage usage, GfxMemoryKind mem, size_t size, const void *data, GfxBuffer **out);
Result gfx_pipeline_create(GfxDevice *dev, const GfxPipelineDesc *desc, GfxPipeline **out);
void gfx_device_destroy(GfxDevice *dev);

GfxFrame *gfx_frame_begin(GfxDevice *dev);
void gfx_bind_pipeline(GfxFrame *f, GfxPipeline *p);
void gfx_draw_indexed(GfxFrame *f, GfxBuffer *vb, GfxBuffer *ib, uint32_t count);
void gfx_frame_end(GfxFrame *f);
