#include "core/core.h"
#include "core/sv.h"
#include <core/array_types.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OBJ_ENTRY_TYPE_V,
    OBJ_ENTRY_TYPE_F,
    OBJ_ENTRY_TYPE_VT,
    OBJ_ENTRY_TYPE_O,
    OBJ_ENTRY_TYPE_INVALID,
    OBJ_ENTRY_TYPE_COUNT,
} ObjEntryType;

typedef struct { float u, v; } ObjVec2;

typedef struct { float x, y, z; } ObjVec3;
typedef struct {
    int32_t poition;
    int32_t texcoord;
    int32_t normal;
} ObjIndex;

DECLARE_ARRAY(ObjVec2);
DECLARE_ARRAY(ObjVec3);
DECLARE_ARRAY(ObjIndex);

typedef struct {
    Array(ObjVec3)  positions;
    Array(ObjVec2)  tex_coords;
    Array(ObjVec3)  normals;
    Array(ObjIndex) indices;
} ObjData;

ObjEntryType asset_obj_entry_type_get(StringView s) {
    StringView entry_types[] = { SV("v"), SV("f"), SV("vt"), SV("o") };
    for (size_t i = 0; i < ARRAY_LEN(entry_types); i++) {
        if (sv_eq(s, entry_types[i])) {
            return i;
        }
    }
    return OBJ_ENTRY_TYPE_INVALID;
}

bool sv_to_float(StringView s, float *out) {
    char buf[64];
    if (s.len == 0 || s.len >= sizeof(buf)) {
        return false;
    }
    memcpy(buf, s.data, s.len);
    buf[s.len] = '\0';
    char *end;
    *out = strtof(buf, &end);
    return end == buf + s.len;
}

Result asset_obj_parse_face(StringView s, Array(ObjIndex) *indices) {
    while (s.len) {
        for (size_t i = 0; i < 3; i++) {
            ObjIndex i = {0};
            StringView tok = sv_trim_left(sv_chop_by_delim(&s, '/'));
            if (!sv_empty(tok)) {
            }
        }
    }
}

ObjData asset_obj_data_init(void) {
    ObjData o = {0};

    tda_create(&o.positions, 1024);
    tda_create(&o.tex_coords, 1024);
    tda_create(&o.normals, 1024);
    tda_create(&o.indices, 1024);
    return (ObjData) {

    };
}

Result asset_obj_file_load(const char *filename, ObjData *out) {
    Array(char) content = {0};
    TRY(read_file(filename, &content));
    StringView file = { tda_data(&content), tda_size(&content) };
    ObjData obj = {0};
    Result r = RESULT_OK;

    while (file.data) {
        StringView line = sv_trim_left(sv_chop_by_delim(&file, '\n'));
        StringView token = sv_trim_left(sv_chop_by_delim(&line, ' '));
        ObjEntryType type = asset_obj_entry_type_get(token);
        switch (type) {
            case OBJ_ENTRY_TYPE_F:
                asset_obj_parse_face(line, &obj.indices);
            case OBJ_ENTRY_TYPE_V:
            case OBJ_ENTRY_TYPE_VT:
            case OBJ_ENTRY_TYPE_O:
            case OBJ_ENTRY_TYPE_INVALID:
            default:
        }
    }
    *out = obj;
fail:
    tda_destroy(&content);
    return r;
}
