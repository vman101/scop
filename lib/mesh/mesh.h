#pragma once

#define MESH

#include "interface/mage_gfx.h"
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "utils/array.h"
#include "geometry/geometry.h"
#include "utils/utils.h"

typedef struct MeshBufferRange     MeshBufferRange;
typedef struct SubMeshDesc         SubMeshDesc;
typedef struct MeshGroup           MeshGroup;
typedef struct SubMesh             SubMesh;
typedef struct Mesh                Mesh;
typedef struct Vertex              Vertex;

DECLARE_ARRAY(Vertex);
DECLARE_ARRAY(MeshGroup);
DECLARE_ARRAY(SubMesh);
DECLARE_ARRAY(SubMeshDesc);

struct       Vertex {
    Vec3     pos;
    Vec3     normal;
    Vec2     uv;
    uint32_t face_id;
};

struct SubMesh {
    uint32_t mtl;
    Range    range;
};

struct MeshGroup {
    char  name[64];
    Range range;
};

typedef struct {
    Array(uint32_t)  indices;
    Array(Vertex)    vertices;
    Array(SubMesh)   submeshes;
    Array(MeshGroup) groups;
} MeshDesc;

struct Mesh {
    char                name[64];
    GfxBuffer           vert_buf;
    GfxBuffer           index_buf;
    Array(SubMesh)      submeshes;
    Array(MeshGroup)    groups;
    GeometryBoundingBox bounding_box;
};

MESH Result              mesh_create(GfxDevice dev, MeshDesc *desc, Mesh *out);
MESH void                mesh_destroy(GfxDevice dev, Mesh *mesh);
MESH void                mesh_draw(GfxFrame f, Mesh *mesh, GfxPushConstantDesc *p);

#undef MESH
