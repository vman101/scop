#pragma once

#define MODEL_API

#include "interface/mage_math.h"
#include "mesh/mesh.h"

typedef struct Model               Model;

struct Model {
    Mesh mesh;
    Vec3 position;
    Vec3 up;
    Mat4 transform;
};

typedef struct {
    Mesh mesh;
} ModelDesc;


MODEL_API Result model_create(GfxDevice dev, ModelDesc *desc, Model *out);
MODEL_API void   model_destroy(GfxDevice dev, Model *model);
MODEL_API void   model_draw(GfxFrame f, GfxPushConstantDesc *pc, Model *m);

#undef MODEL_API
