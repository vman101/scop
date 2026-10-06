#include <stdint.h>
#include <interface/mage_result.h>
#include "utils/array.h"
#include "utils/da.h"
#include "utils/utils.h"
#include "geometry.h"
#include "utils/result_tools.h"

[[maybe_unused]] static Vec2
uv_planar(Vec3 p, GeometryBoundingBox b) {
    Vec3 size = vec3_sub(b.max, b.min);
    return (Vec2) {
        .x = (p.z - b.min.z) / (size.z != 0 ? size.z : 1),
        .y = (p.y - b.min.y) / (size.y != 0 ? size.y : 1),
    };
}

GeometryBoundingBox
geometry_boundry_box_find(const Vec3 *vertices, size_t vertices_count) {
    if (vertices_count == 0) { return (GeometryBoundingBox){0}; }

    GeometryBoundingBox b = { .min = vertices[0], .max = vertices[0] };
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

static Vec2
uv_box(Vec3 p, Vec3 n, GeometryBoundingBox b) {
    Vec3  s     = vec3_sub(b.max, b.min);
    float ax    = fabsf(n.x);
    float ay    = fabsf(n.y);
    float az    = fabsf(n.z);
    float scale = fmaxf(s.x, fmaxf(s.y, s.z));

    if (ax >= ay && ax >= az) return (Vec2){ (p.z - b.min.z) / scale, (p.y - b.min.y) / scale };
    if (ay >= az)             return (Vec2){ (p.x - b.min.x) / scale, (p.z - b.min.z) / scale };
    return                           (Vec2){ (p.x - b.min.x) / scale, (p.y - b.min.y) / scale };
}


Vec3
geometry_calc_normal(Vec3 p[3]) {
    Vec3 n = vec3_normalize(vec3_cross(vec3_sub(p[1], p[0]), vec3_sub(p[2], p[0])));
    return n;
}

Vec2
geometry_calc_uv(Vec3 pos, Vec3 normal, GeometryBoundingBox b) {
    Vec2 uv = {0};
    uv      = uv_box(pos, normal, b);
    uv.y    = 1.0F - uv.y;
    return uv;
}

Vec3
geometry_box_center_get(GeometryBoundingBox b) {
    return vec3_scaler_mul(vec3_add(b.min, b.max), 0.5F);
}


Result
geometry_triangulate(uint32_t *indices, Range range, Array(uint32_t) *out) {
    Result          r             = RESULT_OK;
    Array(uint32_t) tri_indices   = {0};
    size_t          total_indices = 3 * (size_t)(range.count - 2);
    tda_create(&tri_indices, total_indices);

    for (size_t i = range.start; i + 1 < range.count; i++) {
        TRY_GOTO(r, cleanup, tda_push(&tri_indices, &indices[0]));
        TRY_GOTO(r, cleanup, tda_push(&tri_indices, &indices[i]));
        TRY_GOTO(r, cleanup, tda_push(&tri_indices, &indices[i + 1]));
    }

    r = RESULT_OK;
    MOVE(out, tri_indices);
cleanup:
    tda_destroy(&tri_indices);
    printf("DEBUG: face tri_indices count = %zu, my calc %zu\n", tda_size(out), total_indices);
    return r;
}
