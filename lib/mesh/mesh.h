#pragma once

#define MESH

#include "asset/asset.h"
#include "interface/mage_gfx.h"
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "utils/da.h"

typedef struct MeshAxisBoundingBox MeshAxisBoundingBox;
typedef struct MeshBufferRange     MeshBufferRange;
typedef struct MeshGroup           MeshGroup;
typedef struct SubMesh             SubMesh;
typedef struct Mesh                Mesh;
typedef struct Vertex              Vertex;

DECLARE_ARRAY(MeshGroup);
DECLARE_ARRAY(SubMesh);
DECLARE_ARRAY(Vertex);

struct       Vertex {
    Vec3     pos;
    Vec3     normal;
    Vec2     uv;
    uint32_t face_id;
};

struct MeshAxisBoundingBox {
    Vec3    min;
    Vec3    max;
};

struct SubMesh {
    uint32_t       mtl;
    GfxBufferRange range;
};

struct MeshGroup {
    char           name[64];
    GfxBufferRange range;
};

struct Mesh {
    char                name[64];
    GfxBuffer           vert_buf;
    GfxBuffer           index_buf;
    Array(SubMesh)      submeshes;
    Array(MeshGroup)    groups;
    Vec3                position;
    Vec3                up;
    Mat4                transform;
    MeshAxisBoundingBox bounding_box;
};

MESH Result              mesh_create(GfxDevice dev, AssetObjData *obj, Array(AssetMtlLib) *lib, Mesh *out);
MESH void                mesh_destroy(GfxDevice dev, Mesh *mesh);
MESH void                mesh_draw(GfxFrame f, Mesh *mesh, GfxPushConstantDesc *p);
MESH MeshAxisBoundingBox mesh_boundry_box_find(const Vec3 *vertices, size_t vertices_count);
MESH Vec3                mesh_center_get(Mesh *mesh);
