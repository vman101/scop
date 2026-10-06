#include "loader.h"
#include <interface/mage_result.h>
#include "interface/mage_gfx.h"
#include "material/material.h"
#include "model/model.h"
#include "utils/array.h"
#include "asset/asset.h"
#include "mesh/mesh.h"
#include "utils/da.h"
#include "utils/assert.h"
#include <string.h>

static Result
find_mtl_in_lib(const Array(AssetMtl) *mtls, const char *name, size_t *out) {
    Result r = RESULT_OK;
    for (size_t i = 0; i < tda_size(mtls); i++) {
        const AssetMtl *mtl = tda_get(mtls, i);
        if (!mtl) {
            r = RESULT_ERR_NOT_FOUND;
            print_err_with_location(r, "unknown", __FILE__, __LINE__);
            continue;
        }
        if (strcmp(name, mtl->name) == 0) {
            *out = i;
            return RESULT_OK;
        }
    }

    return RESULT_ERR_NOT_FOUND;
}

static Result
collect_mesh_groups_from_obj(const Array(AssetObjGroup) *groups, Array(MeshGroup) *out) {
    Result r = RESULT_OK;
    Array(MeshGroup) mesh_groups = {0};
    for (size_t i = 0; i < tda_size(groups); i++) {
        AssetObjGroup *group = tda_get(groups, i);
        TEST(r, cleanup, group);
        MeshGroup mesh_group = {
            .range = {
                .start = group->range.start,
                .count   = group->range.count,
            },
        };
        TRY_GOTO(r, cleanup, tda_push(&mesh_groups, &mesh_group));
    }
    MOVE(out, mesh_groups);
cleanup:
    tda_destroy(&mesh_groups);
    return r;
}

static Result
collect_submeshes_from_obj(const AssetObj *obj, const Array(AssetMtl) *mtls, Array(SubMesh) *out) {
    Result r = RESULT_OK;
    Array(SubMesh) submeshes = {0};

    for (size_t i = 0; i < tda_size(&obj->use_mtl); i++) {
        AssetObjUseMtl *use = tda_get(&obj->use_mtl, i);
        TEST_GOTO(r, cleanup, use, RESULT_ERR_NOT_FOUND);
        size_t mtl_index = 0;
        TRY_GOTO(r, cleanup, find_mtl_in_lib(mtls, use->name, &mtl_index));
        if (mtl_index == 0) {
            mtl_index = ASSET_DEFAULT_MATERIAL_INDEX;
        }
        SubMesh sub_mesh = {
            .range = use->range,
            .mtl   = mtl_index,
        };
        TRY_GOTO(r, cleanup, tda_push(&submeshes, &sub_mesh));
    }
    if (tda_size(&submeshes) == 0) {
        SubMesh all = {
            .range = { .start = 0, .count = tda_size(&obj->indices) },
            .mtl   = ASSET_DEFAULT_MATERIAL_INDEX,
        };
        TRY_GOTO(r, cleanup, tda_push(&submeshes, &all));
    }
    MOVE(out, submeshes);
cleanup:
    tda_destroy(&submeshes);
    return r;
}

Result
loader_mtls_load(StringView dir, const Array(AssetObjMtlLib) *libs, Array(AssetMtl) *out) {
    Result          r       = RESULT_OK;
    Array(uint8_t)  content = {0};
    Array(AssetMtl) m       = {0};
    Array(AssetMtl) t       = {0};

    for (size_t i = 0; i < tda_size(libs); i++) {
        const AssetObjMtlLib *lib = tda_get(libs, i);
        ASSERT(lib, "lib index in range");

        char mtl_path[PATH_MAX];
        int  len = snprintf(mtl_path, sizeof mtl_path, "%.*s%s%s",
                            (int)dir.len, dir.data, dir.len ? "/" : "", lib->name);
        if (len < 0 || (size_t)len >= sizeof mtl_path) { r = RESULT_ERR_TODO; goto cleanup; }

        TRY_GOTO(r, cleanup, read_file(mtl_path, "r", &content));

        AssetParseDebugTracker tracker = {0};
        r = asset_mtl_data_parse((Array(char) *)&content, &tracker, &t);
        tda_destroy(&content);
        if (r != RESULT_OK) {
            asset_debug_print_tracker(mtl_path, tracker);
            goto cleanup;
        }

        r = tda_append(&m, &t);
        tda_destroy(&t);
        if (r != RESULT_OK) goto cleanup;
    }

    MOVE(out, m);
cleanup:
    tda_destroy(&t);
    tda_destroy(&content);
    tda_destroy(&m);
    return r;
}

Result
loader_obj_load(const char *filepath, AssetObj *out) {
    Result r = RESULT_OK;
    Array(uint8_t) content = {0};
    AssetParseDebugTracker tracker = {0};
    AssetObj       obj     = {0};
    TRY_GOTO(r, cleanup, read_file(filepath, "r", &content));
    r = asset_obj_data_parse((Array(char) *)&content, &tracker, &obj);
    if (r != RESULT_OK) {
        asset_debug_print_tracker(filepath, tracker);
        goto cleanup;
    }
    r = RESULT_OK;
    MOVE(out, obj);
cleanup:
    tda_destroy(&content);
    return r;
}

Result
loader_mesh_build(GfxDevice dev, const AssetObj *obj, const Array(AssetMtl) *mtls, Mesh *out) {
    Mesh   mesh          = {0};
    Result r             = RESULT_OK;
    size_t total_indices = 3 * (size_t)(tda_size(&obj->indices) - 2);

    Array(uint32_t) indices = {0};
    TRY_GOTO(r, cleanup, tda_create(&indices, total_indices));
    Array(Vertex) vertices = {0};
    TRY_GOTO(r, cleanup, tda_create(&vertices, tda_size(&obj->indices)));
    for (size_t i = 0; i < tda_size(&obj->faces); i++) {
        Range *f = tda_get(&obj->faces, i);
        ASSERT(f, "face index in range, can't be NULL");
        ASSERT(f->count >= 3 && f->count <= ASSET_OBJ_FACE_MAX_CORNERS, "bad face size");
        uint32_t corners[ASSET_OBJ_FACE_MAX_CORNERS];
        for (size_t k = 0; k < f->count; k++) {
            AssetObjIndex *idx = tda_get(&obj->indices, f->start + k);
            Vec3 *p = tda_get(&obj->positions, idx->position);
            // Vec3 *n = tda_get(&obj->normals, idx->normal);
            // Vec2 *t = tda_get(&obj->tex_coords, idx->texcoord);
            ASSERT(p != NULL, "position must be present");
            Vertex v = {
                .pos = *p,
            };
            TRY_GOTO(r, cleanup, tda_push(&vertices, &v));
            corners[k] = (uint32_t)(tda_size(&vertices) - 1);
        }

        for (size_t j = 1; j + 1 < f->count; j++) {
            TRY_GOTO(r, cleanup, tda_push(&indices, &corners[0]));
            TRY_GOTO(r, cleanup, tda_push(&indices, &corners[j]));
            TRY_GOTO(r, cleanup, tda_push(&indices, &corners[j + 1]));
        }
    }

    Array(SubMesh) submeshes = {0};
    TRY_GOTO(r, cleanup, collect_submeshes_from_obj(obj, mtls, &submeshes));
    Array(MeshGroup) mesh_groups = {0};
    TRY_GOTO(r, cleanup, collect_mesh_groups_from_obj(&obj->groups, &mesh_groups));
    MeshDesc mesh_desc = {
        .groups = mesh_groups,
        .submeshes = submeshes,
        .indices = indices,
        .vertices = vertices,
    };

    TRY_GOTO(r, cleanup, mesh_create(dev, &mesh_desc, &mesh));
    MOVE(out, mesh);
cleanup:
    tda_destroy(&indices);
    tda_destroy(&vertices);
    mesh_destroy(dev, &mesh);
    return r;
}

Result
loader_model_load(GfxDevice dev, const char *filepath, Model *out) {
    Result          r     = RESULT_OK;
    Model           model = {0};
    AssetObj        obj   = {0};
    Array(AssetMtl) mtls  = {0};
    Mesh            mesh  = {0};

    StringView s    = { filepath, strlen(filepath) };
    StringView path = sv_chop_last(&s, '/');

    TRY_GOTO(r, cleanup, loader_obj_load(filepath, &obj));
    TRY_GOTO(r, cleanup, loader_mtls_load(path, &obj.mtl_lib, &mtls));

    Array(MaterialDesc) m_desc = {0};
    MaterialSet m_set = {0};
    for (size_t i = 0; i < tda_size(&mtls); i++) {
        AssetMtl *m = tda_at(&mtls, i);
        TEST_GOTO(r, cleanup, m, RESULT_ERR_NULL);
        MaterialDesc d = {
            .base_color = { m->diffuse.x, m->diffuse.y, m->diffuse.z, 1.0F },
        };
        TRY_GOTO(r, cleanup, tda_push(&m_desc, &d));
    }

    TRY_GOTO(r, cleanup, material_set_create(dev, &m_desc, &m_set));
    TRY_GOTO(r, cleanup, loader_mesh_build(dev, &obj, &mtls, &mesh));

    r = RESULT_OK;

    ModelDesc desc = {
        .mesh = mesh,
    };

    model_create(dev, &desc, &model);
    MOVE(out, model);
cleanup:
    asset_obj_destroy(&obj);
    tda_destroy(&mtls);
    return r;
}
