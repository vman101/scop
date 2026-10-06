#pragma once
#include <interface/mage_gfx.h>
#include <interface/mage_platform.h>
#include <utils/utils.h>

typedef struct {
    GfxDevice           dev;
    PlatformWindow      window;
    GfxPipeline         pipeline;
    GfxPushConstantDesc pc_vert;
    GfxPushConstantDesc pc_frag;
    GfxPushConstantDesc pc_frag2;
    GfxTexture          tex;
} Scop;

void   on_frame_buffer_resize(void *dev, int width, int height);

Result scop_create(Scop *scop);
void   scop_destroy(Scop *scop);
