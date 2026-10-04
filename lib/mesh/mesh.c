#include "mesh.h"
#include "asset/asset.h"
#include "interface/mage_gfx.h"
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "utils/da.h"
#include "utils/result_tools.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

MESH [[maybe_unused]] static Vec2
uv_planar(Vec3 p, MeshAxisBoundingBox b) {
    Vec3 size = vec3_sub(b.max, b.min);
    return (Vec2) {
        .x = (p.z - b.min.z) / (size.z != 0 ? size.z : 1),
        .y = (p.y - b.min.y) / (size.y != 0 ? size.y : 1),
    };
}

static Vec2 uv_box(Vec3 p, Vec3 n, MeshAxisBoundingBox b) {
    Vec3  s     = vec3_sub(b.max, b.min);
    float ax    = fabsf(n.x);
    float ay    = fabsf(n.y);
    float az    = fabsf(n.z);
    float scale = fmaxf(s.x, fmaxf(s.y, s.z));

    if (ax >= ay && ax >= az) return (Vec2){ (p.z - b.min.z) / scale, (p.y - b.min.y) / scale };
    if (ay >= az)             return (Vec2){ (p.x - b.min.x) / scale, (p.z - b.min.z) / scale };
    return                           (Vec2){ (p.x - b.min.x) / scale, (p.y - b.min.y) / scale };
}

MESH static Result
storage_buffer_create(GfxDevice dev, Array(Material) *materials, GfxBuffer *buf) {
    GfxBuffer new_buf;
    GfxBufferDesc buf_desc = {
        .mem         = GFX_MEMORY_UPLOAD,
        .member_size = sizeof(Material),
        .count       = tda_size(materials),
        .usage       = GFX_BUFFER_STORAGE,
        .data        = tda_data(materials),
    };
    TRY(gfx_buffer_create(dev, &buf_desc, &new_buf));
    gfx_global_buffer_set(dev, GFX_GLOBAL_SLOT_MATERIALS, new_buf);
    *buf = new_buf;

    return RESULT_OK;
}

MESH void
mesh_destroy(GfxDevice dev, Mesh *mesh) {
    tda_destroy(&mesh->groups);
    tda_destroy(&mesh->submeshes);
    gfx_buffer_destroy(dev, mesh->index_buf);
    gfx_buffer_destroy(dev, mesh->vert_buf);
}

MESH static Result
find_mtl_in_lib(Array(AssetMtl) *mtls, const char *name, size_t *out) {
    Result r = RESULT_OK;
    for (size_t i = 0; i < tda_size(mtls); i++) {
        const AssetMtl *mtl = tda_get(mtls, i);
        if (!mtl) {
            r = RESULT_ERR_TODO;
            print_err_with_location(r, "unknown", __FILE__, __LINE__);
            return r;
        }
        if (strcmp(name, mtl->name) == 0) {
            *out = i;
            return RESULT_OK;
        }
    }

    return RESULT_ERR_NOT_FOUND;
}

MESH static Result
find_mtl_in_lib_arr(Array(AssetMtlLib) *mtls, const char *name, size_t *out) {
    Result r = RESULT_OK;
    for (size_t i = 0; i < tda_size(mtls); i++) {
        AssetMtlLib *mtl = tda_get(mtls, i);
        if (!mtl) {
            r = RESULT_ERR_TODO;
            print_err_with_location(r, "unknown", __FILE__, __LINE__);
            return r;
        }
        r = find_mtl_in_lib(&mtl->mtls, name, out);
        if (r == RESULT_OK)            { break; }
        if (r == RESULT_ERR_NOT_FOUND) { continue; }
        break ;
    }

    return r;
}

MESH static size_t
get_full_mtl_count(const Array(AssetMtlLib) *libs) {
    size_t size = 0;
    for (size_t i = 0; i < tda_size(libs); i++) {
        AssetMtlLib *l = tda_at(libs, i);
        if (!l) { break; }
        size += tda_size(&l->mtls);
    }
    return size;
}

MESH static Result
get_submeshes_from_obj_data(AssetObjData *obj, Array(AssetMtlLib) *libs, Array(SubMesh) *out) {
    Result r = RESULT_OK;
    Array(SubMesh) submeshes = {0};

    for (size_t i = 0; i < tda_size(&obj->use_mtl); i++) {
        AssetObjUseMtl *use = tda_get(&obj->use_mtl, i);
        if (!use) { break; }
        size_t mtl_index = 0;
        Result t = find_mtl_in_lib_arr(libs, use->name, &mtl_index);
        if (t != RESULT_OK) {
            if (t == RESULT_ERR_NOT_FOUND) {
                mtl_index = ASSET_DEFAULT_MATERIAL_INDEX;
            } else {
                r = t;
                goto cleanup;
            }
        }
        SubMesh sub_mesh = {
            .range = { .offset = use->index_start, .count  = use->count },
            .mtl   = mtl_index,
        };
        TRY_GOTO(r, cleanup, tda_push(&submeshes, &sub_mesh));
    }
    if (tda_size(&submeshes) == 0) {
        SubMesh all = {
            .range = { .offset = 0, .count = tda_size(&obj->indices) },
            .mtl   = ASSET_DEFAULT_MATERIAL_INDEX,
        };
        TRY_GOTO(r, cleanup, tda_push(&submeshes, &all));
    }
    MOVE(out, submeshes);
cleanup:
    tda_destroy(&submeshes);
    return r;
}

MESH static Result
get_materials_from_mtl_lib_arr(Array(AssetMtlLib) *libs, Array(Material) *out) {
    Result          r         = RESULT_OK;
    Array(Material) materials = {0};

    TRY_GOTO(r, cleanup, tda_create(&materials, get_full_mtl_count(libs) + 1));
    {
        Material m = asset_mtl_material_default();
        TRY_GOTO(r, cleanup, tda_push(&materials, &m));
    }
    for (size_t i = 0; i < tda_size(libs); i++) {
        AssetMtlLib *l = tda_at(libs, i);
        if (!l) { break; }
        for (size_t j = 0; j < tda_size(&l->mtls); j++) {
            AssetMtl *m = tda_get(&l->mtls, j);
            TRY_GOTO(r, cleanup, tda_push(&materials, &m->mtl));
        }
    }

    MOVE(out, materials);
cleanup:
    tda_destroy(&materials);

    return r;
}

MESH Result
mesh_create(GfxDevice dev, AssetObjData *obj, Array(AssetMtlLib) *libs, Mesh *out) {
    assert(obj != NULL);
    Result          r            = RESULT_OK;

    Mesh            mesh         = {0};
    size_t          corner_count = tda_size(&obj->indices);
    Array(Vertex)   vertices     = {0};
    Array(uint32_t) indices      = {0};
    Array(Material) materials    = {0};

    TRY_GOTO(r, cleanup, tda_create(&vertices, corner_count));
    TRY_GOTO(r, cleanup, tda_create(&indices, corner_count));

    mesh.bounding_box = mesh_boundry_box_find(tda_data(&obj->positions), tda_size(&obj->positions));
    for (size_t i = 0; i + 2 < corner_count; i += 3) {
        AssetObjIndex *c[3] = {
            tda_at(&obj->indices, i),
            tda_at(&obj->indices, i + 1),
            tda_at(&obj->indices, i + 2),
        };

        Vec3 p[3];
        for (int k = 0; k < 3; k++) {
            Vec3 *pos = tda_get(&obj->positions, c[k]->position);
            if (!pos) {
                r = RESULT_ERR_TODO;
                print_err_with_location(r, "unknown", __FILE__, __LINE__);
                goto cleanup;
            }
            p[k] = *pos;
        }

        Vec3 face_n = vec3_normalize(vec3_cross(vec3_sub(p[1], p[0]), vec3_sub(p[2], p[0])));
        for (int k = 0; k < 3; k++) {
            Vertex v = { .pos = p[k] };
            if (c[k]->normal != ASSET_OBJ_NO_VALUE) {
                Vec3 *n = tda_get(&obj->normals, c[k]->normal);
                if (!n) {
                    r = RESULT_ERR_TODO;
                    print_err_with_location(r, "unknown", __FILE__, __LINE__);
                    goto cleanup;
                }
                v.normal = *n;
            } else {
                v.normal = face_n;
            }
            if (c[k]->texcoord != ASSET_OBJ_NO_VALUE) {
                Vec2 *t = tda_get(&obj->tex_coords, c[k]->texcoord);
                if (!t) {
                    r = RESULT_ERR_TODO;
                    print_err_with_location(r, "unknown", __FILE__, __LINE__);
                    goto cleanup;
                }
                v.uv = *t;
            } else {
                v.uv = uv_box(v.pos, v.normal, mesh.bounding_box);
            }
            v.uv.y = 1.0F - v.uv.y;
            uint32_t new_index = (uint32_t)tda_size(&vertices);
            v.face_id = c[0]->face_id;
            TRY_GOTO(r, cleanup, tda_push(&vertices, &v));
            TRY_GOTO(r, cleanup, tda_push(&indices, &new_index));
        }
    }

    GfxBufferDesc vert_buf_desc = {
        .count       = tda_size(&vertices),
        .member_size = tda_sizeof(&vertices),
        .data        = tda_data(&vertices),
        .mem         = GFX_MEMORY_UPLOAD,
        .usage       = GFX_BUFFER_VERTEX,
    };
    TRY_GOTO(r, cleanup, gfx_buffer_create(dev, &vert_buf_desc, &mesh.vert_buf));

    GfxBufferDesc idx_buf_desc = {
        .count       = tda_size(&indices),
        .member_size = tda_sizeof(&indices),
        .data        = tda_data(&indices),
        .mem         = GFX_MEMORY_UPLOAD,
        .usage       = GFX_BUFFER_INDEX,
    };
    TRY_GOTO(r, cleanup, gfx_buffer_create(dev, &idx_buf_desc, &mesh.index_buf));

    for (size_t i = 0; i < tda_size(&obj->groups); i++) {
        AssetObjGroup *group = tda_get(&obj->groups, i);
        if (!group) break;
        MeshGroup mesh_group = {
            .range = {
                .offset = group->index_start,
                .count  = group->count,
            },
        };
        TRY_GOTO(r, cleanup, tda_push(&mesh.groups, &mesh_group));
    }

    TRY_GOTO(r, cleanup, get_materials_from_mtl_lib_arr(libs, &materials));
    TRY_GOTO(r, cleanup, get_submeshes_from_obj_data(obj, libs, &mesh.submeshes));

    GfxBuffer buf = {0};
    TRY_GOTO(r, cleanup, storage_buffer_create(dev, &materials, &buf));

    MOVE(out, mesh);

    r = RESULT_OK;
cleanup:
    tda_destroy(&indices);
    tda_destroy(&vertices);
    tda_destroy(&materials);

    mesh_destroy(dev, &mesh);
    return r;
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
mesh_draw(GfxFrame f, Mesh *mesh, GfxPushConstantDesc *p) {
    for (size_t i = 0; i < tda_size(&mesh->submeshes); i++) {
        SubMesh *m = tda_get(&mesh->submeshes, i);
        gfx_push_constant(f, p, &m->mtl);
        gfx_draw_indexed_range(f, mesh->vert_buf, mesh->index_buf, m->range);
    }
}

MESH Vec3
mesh_center_get(Mesh *mesh) {
    return vec3_scaler_mul(vec3_add(mesh->bounding_box.min, mesh->bounding_box.max), 0.5F);
}
