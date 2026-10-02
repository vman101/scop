#include "mesh.h"
#include "asset/asset.h"
#include "interface/mage_gfx.h"
#include "interface/mage_math.h"
#include "utils/da.h"
#include "utils/result_tools.h"
#include <math.h>

MESH static Result
mesh_asset_obj_indices_get(AssetObjData *obj, Array(uint32_t) *indices) {
    TRY(tda_create(indices, tda_size(&obj->indices)));
    for (size_t i = 0; i < tda_size(&obj->indices); ++i) {
        AssetObjIndex *index = tda_at(&obj->indices, i);
        uint32_t pos = index->position - 1;
        TRY(tda_push(indices, &pos));
    }
    return RESULT_OK;
}

MESH Result
mesh_load_from_obj(GfxDevice dev, const char *filename, Mesh *out) {
    Mesh                   mesh    = {0};
    AssetObjData           obj     = {0};
    AssetParseDebugTracker tracker = {0};
    TRY(asset_obj_file_load(filename, &tracker, &obj));

    Array(Vec3) *positions = &obj.positions;

    GfxBufferDesc vert_buf_desc = {
        .count = tda_size(positions),
        .member_size = tda_sizeof(positions),
        .data = tda_data(positions),
        .mem = GFX_MEMORY_UPLOAD,
        .usage = GFX_BUFFER_VERTEX,
    };

    TRY(gfx_buffer_create(dev, &vert_buf_desc, &mesh.vert_buf));
    Array(uint32_t) indices = {0};
    TRY(mesh_asset_obj_indices_get(&obj, &indices));

    GfxBufferDesc idx_buf_desc = {
        .count = tda_size(&indices),
        .member_size = tda_sizeof(&indices),
        .data = tda_data(&indices),
        .mem = GFX_MEMORY_UPLOAD,
        .usage = GFX_BUFFER_INDEX,
    };
    TRY(gfx_buffer_create(dev, &idx_buf_desc, &mesh.index_buf));

    for (size_t i = 0; i < tda_size(&obj.groups); i++) {
        AssetObjGroup *group = tda_get(&obj.groups, i);
        if (!group) break;
        SubMesh sub_mesh = {
            .range = {
                .start = group->first_index,
                .count = group->index_count,
            },
        };
        TRY(tda_push(&mesh.submeshes, &sub_mesh));
    }
    mesh.bounding_box = mesh_boundry_box_find(tda_data(&obj.positions), tda_size(&obj.positions));
    *out = mesh;
    return RESULT_OK;
}

MESH MeshAxisBoundingBox
mesh_boundry_box_find(const Vec3 *vertices, size_t vertices_count) {
    if (vertices_count == 0) { return (MeshAxisBoundingBox){0}; }

    MeshAxisBoundingBox b = { .min = vertices[0], .max = vertices[0] };
    for (size_t i = 0; i < vertices_count; i++) {
        b.min.x = fminf(b.min.x, vertices[i].x);
        b.min.y = fminf(b.min.y, vertices[i].y);
        b.min.z = fminf(b.min.z, vertices[i].z);
        b.max.x = fmaxf(b.max.x, vertices[i].x);
        b.max.y = fmaxf(b.max.y, vertices[i].y);
        b.max.z = fmaxf(b.max.z, vertices[i].z);
    }
    return b;
}

MESH void
mesh_draw(GfxFrame f, Mesh *mesh) {
    gfx_draw_indexed(f, mesh->vert_buf, mesh->index_buf);
}

MESH Vec3
mesh_center_get(Mesh *mesh) {
    return vec3_scaler_mul(vec3_add(mesh->bounding_box.min, mesh->bounding_box.max), 0.5F);
}
