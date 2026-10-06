#pragma once
#include "interface/mage_gfx.h"
#include "utils/array.h"
#include "utils/result_tools.h"

#define MATERIAL_API
typedef struct Material           Material;
typedef struct MaterialDesc       MaterialDesc;
typedef struct MaterialParamsDesc MaterialParamsDesc;

DECLARE_ARRAY(Material);
DECLARE_ARRAY(MaterialDesc);
DECLARE_ARRAY(MaterialParamsDesc);

#define MATERIAL_FLAGS                    \
    X(MATERIAL_FLAG_NONE, 0)              \
    X(MATERIAL_FLAG_TRANSPARENT, 1 << 0)  \
    X(MATERIAL_FLAG_DOUBLE_SIDED, 2 << 1) \

#define MATERIAL_TEXTURE_SLOTS     \
    X(MATERIAL_TEXTURE_BASE_COLOR) \
    X(MATERIAL_TEXTURE_NORMAL)     \
    X(MATERIAL_TEXTURE_SPECULAR)   \
    X(MATERIAL_TEXTURE_COUNT) \

typedef enum {
#define X(name, val) name = (val),
    MATERIAL_FLAGS
#undef X
} MaterialFlags;

typedef enum {
#define X(name) name,
    MATERIAL_TEXTURE_SLOTS
#undef X
} MaterialTextureSlot;

struct MaterialParamsDesc {
    Vec4    base_color;
};

struct            MaterialDesc {
    Vec4          base_color;
    GfxTexture    base_color_tex;
    MaterialFlags flags;
};

struct            Material {
    Vec4          base_color;
    GfxTexture    textures[MATERIAL_TEXTURE_COUNT];
    size_t        index;
    MaterialFlags flags;
};

typedef struct {
    Array(Material) materials;
    GfxBuffer       params_buf;
} MaterialSet;

MATERIAL_API Result material_set_create(GfxDevice dev, const Array(MaterialDesc) *desc, MaterialSet *out);
MATERIAL_API void   material_set_destroy(GfxDevice dev, MaterialSet *mat);
