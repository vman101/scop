#pragma once

#include <interface/mage_math.h>
#include <stdint.h>
#include "utils/utils.h"

typedef struct {
    Vec3    min;
    Vec3    max;
} GeometryBoundingBox;


Vec3 geometry_calc_normal(Vec3 p[3]);
Vec2 geometry_calc_uv(Vec3 pos, Vec3 normal, GeometryBoundingBox b);
Vec3 geometry_box_center_get(GeometryBoundingBox b);
Result geometry_triangulate(uint32_t *indices, Range range, Array(uint32_t) *out);
