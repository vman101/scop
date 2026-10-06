#include "interface/mage_result.h"
#include "utils/da.h"
#include "utils/sv.h"
#include <utils/array.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "asset.h"
#include <interface/mage_math.h>
#include "utils/result_tools.h"
#include "utils/utils.h"
#include <utils/defines.h>

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

void
asset_obj_destroy(AssetObj *obj) {
    if (!obj) return;
    tda_destroy(&obj->positions);
    tda_destroy(&obj->indices);
    tda_destroy(&obj->normals);
    tda_destroy(&obj->groups);
    tda_destroy(&obj->use_mtl);
    tda_destroy(&obj->mtl_lib);
    tda_destroy(&obj->tex_coords);
}

Result
asset_obj_parse_face_field(StringView field, AssetParseDebugTracker *tracker, AssetObjIndex *indices) {
    const char *const start_pos = field.data;
    StringView s                = sv_trim_left(sv_chop(&field, ' '));
    int32_t values[3]           = {0};

    while (s.len) {
        for (size_t i = 0; i < 3; i++) {
            StringView tok = sv_trim_left(sv_chop(&s, '/'));
            asset_debug_parser_tracker_cursor_advance(tracker, tok.data - start_pos);
            if (sv_empty(&tok)) { 
                values[i] = ASSET_OBJ_NO_VALUE;
                continue;
            }
            TRY(sv_to_long(tok, &values[i]));
            values[i] -= 1;
        }
    }
    *indices = ((AssetObjIndex){ values[0], values[1], values[2], (int32_t)tracker->line });
    return RESULT_OK;
}

static void
asset_obj_group_close(AssetObj *o) {
    AssetObjGroup *last = tda_back(&o->groups);
    if (last) {
        last->range.count = tda_size(&o->indices) - last->range.start;
    }
}

Result
asset_obj_parse_face(StringView line, AssetParseDebugTracker *tracker, AssetObj *o) {
    AssetObjIndex verts[64];
    size_t n = 0;
    Range  face_range = {
        .start = tda_size(&o->indices),
        .count = 0,
    };

    while (line.len) {
        StringView field = sv_trim_left(sv_chop(&line, ' '));
        if (sv_empty(&field)) continue;
        if (n == 64) return RESULT_ERR_PARSE_EXPECT;
        TRY(asset_obj_parse_face_field(field, tracker, &verts[n++]));
    }
    if (n < 3) return RESULT_ERR_PARSE_EXPECT;
    face_range.count = n;
    for (size_t i = 0; i < ARRAY_LEN(verts); i++) {
        TRY(tda_push(&o->indices, &verts[i]));
    }
    TRY(tda_push(&o->faces, &face_range));
    return RESULT_OK;
}

Result
asset_obj_parse_group(StringView line, AssetParseDebugTracker *tracker, AssetObj *obj) {
    asset_obj_group_close(obj);
    AssetObjGroup new = {0};
    TRY(asset_parse_string(line, tracker, new.name));
    new.range.start = tda_size(&obj->indices);
    new.range.count = 0;
    TRY(tda_push(&obj->groups, &new));
    return RESULT_OK;
}

Result
asset_obj_parse_mtllib(StringView line, AssetParseDebugTracker *tracker, AssetObj *obj) {
    const char *start = line.data;
    while (!sv_empty(&line)) {
        StringView mtl = sv_trim_left(sv_chop(&line, ' '));
        asset_debug_parser_tracker_cursor_advance(tracker, mtl.data - start);
        if (sv_empty(&mtl)) continue;
        AssetObjMtlLib lib = {0};
        strncat(lib.name, mtl.data, mtl.len);
        TRY(tda_push(&obj->mtl_lib, &lib));
    }
    return RESULT_OK;
}

Result
asset_obj_parse_use_mtl(StringView line, AssetParseDebugTracker *tracker, AssetObj *obj) {
    char mtl_name[64] = {0};
    StringView tok = sv_trim_left(sv_chop(&line, ' '));
    asset_debug_parser_tracker_cursor_advance(tracker, line.data - tok.data);
    sv_strcopy(tok, mtl_name);
    AssetObjUseMtl *mtl = tda_back(&obj->use_mtl);
    if (mtl) {
        mtl->range.count = tda_size(&obj->indices) - mtl->range.start;
    }

    AssetObjUseMtl new_use = {
        .range = {
            .start = tda_size(&obj->indices),
            .count = 0,
        }
    };
    sv_strcopy(tok, new_use.name);
    TRY(tda_push(&obj->use_mtl, &new_use));

    return RESULT_OK;
}

Result
asset_obj_data_parse(const Array(char) *content, AssetParseDebugTracker *tracker, AssetObj *out) {
    Result        r     = RESULT_OK;
    StringView    file  = { tda_data(content), tda_size(content) };
    AssetObjGroup group = {0};
    group.range.start   = 0;
    tda_push(&out->groups, &group);

    while (file.len) {
        asset_debug_parser_tracker_line_advance(tracker, 1);
        StringView line   = sv_trim(sv_chop(&file, '\n'));
        if (sv_empty(&line) || line.data[0] == '#') { continue; }
        StringView token  = sv_chop(&line, ' ');

        line = sv_trim_left(line);
        AssetObjEntryType type = asset_entry_type_get(token, entry_types_sv, (int *)entry_types_ls, ARRAY_LEN(entry_types_sv));
        switch (type) {
            case ASSET_OBJ_ENTRY_TYPE_F:
                TRY_GOTO(r, fail, asset_obj_parse_face(line, tracker, out));
                break;
            case ASSET_OBJ_ENTRY_TYPE_V:
                TRY_GOTO(r, fail, asset_parse_vec3_into_arr(line, tracker, &out->positions));
                break;
            case ASSET_OBJ_ENTRY_TYPE_VN:
                TRY_GOTO(r, fail, asset_parse_vec3_into_arr(line, tracker, &out->normals));
                break;
            case ASSET_OBJ_ENTRY_TYPE_VT:
                TRY_GOTO(r, fail, asset_parse_vec2_into_arr(line, tracker, &out->tex_coords));
                break;
            case ASSET_OBJ_ENTRY_TYPE_MTL_LIB:
                TRY_GOTO(r, fail, asset_obj_parse_mtllib(line, tracker, out));
                break;
            case ASSET_OBJ_ENTRY_TYPE_USE_MTL:
                TRY_GOTO(r, fail, asset_obj_parse_use_mtl(line, tracker, out));
                break;
            case ASSET_OBJ_ENTRY_TYPE_S:
                break;
            case ASSET_OBJ_ENTRY_TYPE_O:
            case ASSET_OBJ_ENTRY_TYPE_G:
                TRY_GOTO(r, fail, asset_obj_parse_group(line, tracker, out));
            case ASSET_OBJ_ENTRY_TYPE_COMMENT:
                break;
            case ASSET_OBJ_ENTRY_TYPE_INVALID:
            default:
                asset_debug_print_invalid_token(NULL, tracker, token, type, entry_types_nm);
        }
    }
fail:
    {
        AssetObjUseMtl *back = tda_back(&out->use_mtl);
        if (back && back->range.start < tda_size(&out->indices) && back->range.count == 0) {
            back->range.count = tda_size(&out->indices) - back->range.start;
        }
        for (size_t i = 0; i < tda_size(&out->groups); i++) {
            AssetObjGroup *cur = tda_get(&out->groups, i);
            if (cur && cur->range.count == 0) {
                tda_remove(&out->groups, i);
            }
        }
        asset_obj_group_close(out);
    }
    return r;
}
