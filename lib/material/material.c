#include "material.h"
#include "utils/assert.h"
#include "utils/da.h"
#include <interface/mage_gfx.h>

void
material_set_destroy(GfxDevice dev, MaterialSet *mat) {
    if (!mat) { return; }
    tda_destroy(&mat->materials);
    gfx_buffer_destroy(dev, mat->params_buf);
}

Result
material_set_create(GfxDevice dev, const Array(MaterialDesc) *desc, MaterialSet *out) {
    ASSERT(desc, "desc is null");
    Result r = RESULT_OK;
    MaterialSet mat_set = {0};

    Array(MaterialParamsDesc) params = {0};
    TRY_GOTO(r, cleanup, tda_create(&params, tda_size(desc)));
    TRY_GOTO(r, cleanup, tda_create(&mat_set.materials, tda_size(desc)));

    for (size_t i = 0; i < tda_size(desc); i++) {
        MaterialDesc *d = tda_get(desc, i);
        TEST(r, cleanup, d);

        Material mat   = {0};
        mat.base_color = d->base_color;
        mat.flags      = d->flags;
        mat.index      = i;
        MaterialParamsDesc p = {
            .base_color = mat.base_color,
        };
        TRY_GOTO(r, cleanup, tda_push(&params, &p));
    }

    GfxBufferDesc buf_desc = {
        .mem         = GFX_MEMORY_UPLOAD,
        .count       = tda_size(&params),
        .member_size = tda_sizeof(&params),
        .data        = tda_data(&params),
        .usage       = GFX_BUFFER_STORAGE,
    };

    TRY_GOTO(r, cleanup, gfx_buffer_create(dev, &buf_desc, &mat_set.params_buf));
    gfx_global_buffer_set(dev, GFX_GLOBAL_SLOT_MATERIALS, mat_set.params_buf);

    MOVE(out, mat_set);
    r = RESULT_OK;
cleanup:
    tda_destroy(&params);
    tda_destroy(&mat_set.materials);
    material_set_destroy(dev, &mat_set);
    return r;
}
