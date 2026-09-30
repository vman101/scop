#include "core/result.h"
#include "utils/sv.h"
#include <utils/array_types.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "asset.h"
#include <core/math.h>
#include "utils/result_tools.h"
#include "utils/utils.h"

const StringView entry_types_sv[] = {
#define X(name, tok) SV(tok),
    ASSET_OBJ_ENTRY_LIST
#undef X
};

const AssetObjEntryType entry_types_ls[] = {
#define X(name, tok) name,
    ASSET_OBJ_ENTRY_LIST
#undef X
};

const char *entry_types_nm[] = {
#define X(name, tok) #name,
    ASSET_OBJ_ENTRY_LIST
#undef X
};

Result asset_obj_parse_face_field(StringView field, AssetParseDebugTracker *tracker, AssetObjIndex *indices) {
    const char *const start_pos = field.data;
    StringView s                = sv_trim_left(sv_chop_by_delim(&field, ' '));
    int32_t values[3]           = {0};

    while (s.len) {
        for (size_t i = 0; i < 3; i++) {
            StringView tok = sv_trim_left(sv_chop_by_delim(&s, '/'));
            tracker->cursor = tok.data - start_pos;
            if (sv_empty(&tok)) { continue; }
            TRY(sv_to_long(tok, &values[i]));
        }
    }
    *indices = ((AssetObjIndex){ values[0], values[1], values[2] });
    return RESULT_OK;
}

static void asset_obj_group_close(AssetObjData *o) {
    AssetObjGroup *last = tda_back(&o->groups);
    if (last) {
        last->index_count = tda_size(&o->indices) - last->first_index;
    }
}

Result asset_obj_parse_face(StringView line, AssetParseDebugTracker *tracker, AssetObjData *o) {
    AssetObjIndex verts[64];
    size_t n = 0;

    while (line.len) {
        StringView field = sv_trim_left(sv_chop_by_delim(&line, ' '));
        if (sv_empty(&field)) continue;
        if (n == 64) return RESULT_ERR_PARSE_EXPECT;              // absurd polygon
        TRY(asset_obj_parse_face_field(field, tracker, &verts[n++]));
    }
    if (n < 3) return RESULT_ERR_PARSE_EXPECT;

    for (size_t i = 1; i + 1 < n; i++) {
        tda_push(&o->indices, &verts[0]);
        tda_push(&o->indices, &verts[i]);
        tda_push(&o->indices, &verts[i + 1]);
    }
    return RESULT_OK;
}

AssetObjData asset_obj_data_init(void) {
    AssetObjData o = {0};

    tda_create(&o.positions, 1024);
    tda_create(&o.tex_coords, 1024);
    tda_create(&o.normals, 1024);
    tda_create(&o.indices, 1024);
    return o;
}

Result asset_obj_parse_position(StringView line, AssetParseDebugTracker *tracker, Array(Vec3) *positions) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 3, &v));
    tda_push(positions, &v.v3);
    return RESULT_OK;
}

Result asset_obj_parse_tex_coords(StringView line, AssetParseDebugTracker *tracker, Array(Vec2) *tex_coords) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 2, &v));
    tda_push(tex_coords, &v.v2);
    return RESULT_OK;
}

Result asset_obj_parse_group(StringView line, AssetParseDebugTracker *tracker, AssetObjData *obj) {
    asset_obj_group_close(obj);
    AssetObjGroup new = {0};
    TRY(asset_parse_string(line, tracker, new.name));
    new.first_index = tda_size(&obj->indices);
    new.index_count = 0;
    return RESULT_OK;
}

Result asset_obj_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetObjData *out) {
    Array(char) content = {0};
    TRY(read_file(filename, "r", &content));
    StringView file = { tda_data(&content), tda_size(&content) };
    Result r = RESULT_OK;

    while (file.len) {
        tracker->line++;
        StringView line   = sv_trim_left(sv_chop_by_delim(&file, '\n'));
        if (sv_empty(&line)) { continue; }
        StringView token  = sv_trim_left(sv_chop_by_delim(&line, ' '));
        AssetObjEntryType type = asset_entry_type_get(token, entry_types_sv, (int *)entry_types_ls, ARRAY_LEN(entry_types_sv));
        switch (type) {
            case ASSET_OBJ_ENTRY_TYPE_F:
                TRY_GOTO(r, fail, asset_obj_parse_face(line, tracker, out));
                break;
            case ASSET_OBJ_ENTRY_TYPE_V:
                TRY_GOTO(r, fail, asset_obj_parse_position(line, tracker, &out->positions));
                break;
            case ASSET_OBJ_ENTRY_TYPE_VT:
                TRY_GOTO(r, fail, asset_obj_parse_tex_coords(line, tracker, &out->tex_coords));
                break;
            case ASSET_OBJ_ENTRY_TYPE_MTL_LIB:
            case ASSET_OBJ_ENTRY_TYPE_S:
            case ASSET_OBJ_ENTRY_TYPE_USE_MTL:
                printf("TODO: Impl parsing for type %s\n", entry_types_nm[type]);
            case ASSET_OBJ_ENTRY_TYPE_O:
            case ASSET_OBJ_ENTRY_TYPE_G:
                TRY_GOTO(r, fail, asset_obj_parse_group(line, tracker, out));
            case ASSET_OBJ_ENTRY_TYPE_COMMENT:
                break;
            case ASSET_OBJ_ENTRY_TYPE_INVALID:
            default:
                asset_debug_print_invalid_token(filename, tracker, token, type, entry_types_nm);
        }
    }
fail:
    tda_destroy(&content);
    return r;
}
