#pragma once

#include "interface/mage_gfx.h"
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "utils/da.h"

typedef struct MeshAxisBoundingBox MeshAxisBoundingBox;
typedef struct MeshBufferRange     MeshBufferRange;
typedef struct SubMesh             SubMesh;
typedef struct Mesh                Mesh;

#define MESH
DECLARE_ARRAY(SubMesh);

struct MeshAxisBoundingBox {
    Vec3    min;
    Vec3    max;
};

struct MeshBufferRange {
    uint32_t    start;
    uint32_t    count;
};

struct SubMesh {
    MeshBufferRange range;
};

struct Mesh {
    char                name[64];
    GfxBuffer           vert_buf;
    GfxBuffer           index_buf;
    Array(SubMesh)      submeshes;
    Mat4                transform;
    MeshAxisBoundingBox bounding_box;
};

MESH Result                 mesh_load_from_obj(GfxDevice dev, const char *filename, Mesh *out);
MESH void                   mesh_draw(GfxFrame f, Mesh *mesh);
MESH MeshAxisBoundingBox    mesh_boundry_box_find(const Vec3 *vertices, size_t vertices_count);
MESH Vec3                   mesh_center_get(Mesh *mesh);
