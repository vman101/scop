#pragma once
#include <stdint.h>
#include <core/result.h>

#define GFX_MAX_BINDINGS    5
#define GFX_MAX_ATTRIBUTES  5

typedef struct GfxDeviceDesc            GfxDeviceDesc;
typedef void                            GfxWindow;

typedef struct GfxDevice_T              GfxDevice_T;
typedef struct GfxBuffer_T              GfxBuffer_T;
typedef struct GfxPipeline_T            GfxPipeline_T;
typedef struct GfxFrame_T               GfxFrame_T;
typedef struct GfxVertexLayout_T        GfxVertexLayout_T;
typedef struct GfxDevice_T *            GfxDevice;
typedef struct GfxBuffer_T *            GfxBuffer;
typedef struct GfxPipeline_T *          GfxPipeline;
typedef struct GfxFrame_T *             GfxFrame;

typedef enum { GFX_BUFFER_VERTEX, GFX_BUFFER_INDEX, GFX_BUFFER_UNIFORM } GfxBufferUsage;
typedef enum { GFX_MEMORY_GPU, GFX_MEMORY_UPLOAD, GFX_MEMORY_READBACK } GfxMemoryKind;

typedef enum {
    GFX_FORMAT_FLOAT,
    GFX_FORMAT_FLOAT2,
    GFX_FORMAT_FLOAT3,
    GFX_FORMAT_FLOAT4,
    GFX_FORMAT_UBYTE4_NORM,
} GfxFormat;

typedef struct {
    uint32_t  location;
    GfxFormat format;
    uint32_t  offset;
} GfxVertexAttribute;

typedef struct {
    uint32_t           stride;
    bool               per_instance;
    GfxVertexAttribute attributes[GFX_MAX_ATTRIBUTES];
    uint32_t           attribute_count;
} GfxVertexBinding;

typedef struct {
    GfxVertexBinding bindings[GFX_MAX_BINDINGS];
    uint32_t         binding_count;
} GfxVertexLayout;

typedef struct {
    const char      *vertex_shader_path;
    const char      *fragment_shader_path;
    bool             depth_test;
    GfxVertexLayout  vertex_layout;
} GfxPipelineDesc;

Result gfx_device_create(GfxDeviceDesc *dev_info, GfxDevice *dev);
Result gfx_buffer_create(GfxDevice dev, GfxBufferUsage usage, GfxMemoryKind mem, size_t size, const void *data, GfxBuffer *out);
Result gfx_pipeline_create(GfxDevice dev, GfxPipelineDesc *desc);
void gfx_device_destroy(GfxDevice dev);

GfxFrame *gfx_frame_begin(GfxDevice dev);
void gfx_bind_pipeline(GfxFrame *f, GfxPipeline *p);
void gfx_draw_indexed(GfxFrame *f, GfxBuffer *vb, GfxBuffer *ib, uint32_t count);
void gfx_frame_end(GfxFrame *f);
