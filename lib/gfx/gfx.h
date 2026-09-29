#pragma once
#include "core/core.h"
#include <stdint.h>
#include <core/result.h>
#include <core/native_window.h>
#include <core/array_types.h>

#define GFX_MAX_BINDINGS    5
#define GFX_MAX_ATTRIBUTES  5

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
    const char          *vertex_shader_path;
    const char          *fragment_shader_path;
    bool                depth_test;
    GfxVertexLayout     vertex_layout;
} GfxPipelineDesc;

typedef struct {
    GfxBufferUsage  usage;
    GfxMemoryKind   mem;
    uint64_t        count;
    uint64_t        member_size;
    const void      *data;
} GfxBufferDesc;

typedef void * GfxPlatform;
typedef void * GfxInstance;
typedef void * GfxSurface;

typedef struct {
    const char          *app_name;
    CoreNativeWindow    *window;
    bool                debug_mode;
    uint32_t            width;
    uint32_t            height;
} GfxDeviceDesc;

void gfx_resize(GfxDevice dev, int32_t width, int32_t height);

Result gfx_device_create(GfxDeviceDesc *dev_info, GfxDevice *dev);
Result gfx_buffer_create(GfxDevice dev, GfxBufferDesc *desc, GfxBuffer *out);
Result gfx_pipeline_create(GfxDevice dev, GfxPipelineDesc *desc, GfxPipeline *out);
void gfx_device_destroy(GfxDevice dev);

Result gfx_frame_begin(GfxDevice dev, GfxFrame *out);
void gfx_pass_begin(GfxFrame frame, const float clear[4]);
void gfx_bind_pipeline(GfxFrame f, GfxPipeline p);
void gfx_draw(GfxFrame frame, GfxBuffer vertices);
void gfx_draw_indexed(GfxFrame frame, GfxBuffer vertices, GfxBuffer indices);
void gfx_pass_end(GfxFrame frame);
Result gfx_frame_end(GfxFrame f);

Result gfx_platform_surface_instance_extensions(Array(CharPtr) *out, uint32_t *ws_mask);
void gfx_platform_framebuffer_size_get(GfxDevice dev, const CoreNativeWindow *window);
Result gfx_platform_surface_create(GfxDevice dev, uint32_t ws_mask, const CoreNativeWindow *w);
