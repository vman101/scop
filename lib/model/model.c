#include "model.h"
#include "interface/mage_math.h"
#include "interface/mage_result.h"
#include "mesh/mesh.h"
#include "utils/assert.h"
#include "utils/result_tools.h"
#include <assert.h>

void
model_destroy(GfxDevice dev, Model *model) {
    mesh_destroy(dev, &model->mesh);
}

Result
model_create(GfxDevice dev, ModelDesc *desc, Model *out) {
    (void)dev;
    ASSERT(desc, "desc is null");
    ASSERT(out, "out is null");

    Result r = RESULT_OK;
    Model model = {0};

    model.transform = mat4_identity();
    model.position = vec3(0.0F, 0.0F, 0.0F);
    model.mesh = desc->mesh;

    r = RESULT_OK;
    MOVE(out, model);
    return r;
}

void
model_draw(GfxFrame f, GfxPushConstantDesc *pc, Model *m) {
    mesh_draw(f, &m->mesh, pc);
}
