#include "mesh.h"
#include "interface/mage_gfx.h"
#include "utils/da.h"
#include "utils/result_tools.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

Result
mesh_create(GfxDevice dev, MeshDesc *desc, Mesh *out) {
    Result             r            = RESULT_OK;
    Mesh               mesh         = {0};
    Array(Vertex)      vertices     = desc->vertices;
    Array(uint32_t)    indices      = desc->indices;

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

    tda_create(&mesh.submeshes, tda_size(&desc->submeshes));
    for (size_t i = 0; i < tda_size(&desc->submeshes); i++) {
        SubMesh *d = tda_get(&desc->submeshes, i);
        TEST_GOTO(r, cleanup, d, RESULT_ERR_NOT_FOUND);
        SubMesh sm = {
            .mtl = d->mtl,
            .range = d->range,
        };
        TRY_GOTO(r, cleanup, tda_push(&mesh.submeshes, &sm));
    }

    MOVE(out, mesh);

    r = RESULT_OK;
cleanup:

    mesh_destroy(dev, &mesh);
    return r;
}

void
mesh_draw(GfxFrame f, Mesh *mesh, GfxPushConstantDesc *p) {
    for (size_t i = 0; i < tda_size(&mesh->submeshes); i++) {
        SubMesh *m = tda_get(&mesh->submeshes, i);
        gfx_push_constant(f, p, &m->mtl);
        gfx_draw_range(f, m->range.start, m->range.count);
    }
}

void
mesh_destroy(GfxDevice dev, Mesh *mesh) {
    tda_destroy(&mesh->groups);
    tda_destroy(&mesh->submeshes);
    gfx_buffer_destroy(dev, mesh->index_buf);
    gfx_buffer_destroy(dev, mesh->vert_buf);
}
