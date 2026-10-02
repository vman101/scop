#include "interface/mage_result.h"
#include "utils/da.h"
#include "utils/sv.h"
#include <utils/array_types.h>
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

Result asset_obj_parse_face_field(StringView field, AssetParseDebugTracker *tracker, AssetObjIndex *indices) {
    const char *const start_pos = field.data;
    StringView s                = sv_trim_left(sv_chop(&field, ' '));
    int32_t values[3]           = {0};

    while (s.len) {
        for (size_t i = 0; i < 3; i++) {
            StringView tok = sv_trim_left(sv_chop(&s, '/'));
            asset_debug_parser_tracker_cursor_advance(tracker, tok.data - start_pos);
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
        StringView field = sv_trim_left(sv_chop(&line, ' '));
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

Result asset_obj_parse_group(StringView line, AssetParseDebugTracker *tracker, AssetObjData *obj) {
    asset_obj_group_close(obj);
    AssetObjGroup new = {0};
    TRY(asset_parse_string(line, tracker, new.name));
    new.first_index = tda_size(&obj->indices);
    new.index_count = 0;
    return RESULT_OK;
}

Result asset_obj_parse_and_load_mtllib(StringView line, AssetParseDebugTracker *tracker, AssetObjData *obj) {
    while (!sv_empty(&line)) {
        StringView mtl = sv_trim_left(sv_chop(&line, ' '));
        if (sv_empty(&mtl)) continue;
        char filename[PATH_MAX];
        strcpy(filename, obj->path);
        strcat(filename, "/");
        strncat(filename, mtl.data, mtl.len);
        TRY(asset_mtl_file_load(filename, tracker, &obj->materials));
    }
    return RESULT_OK;
}

Result asset_obj_parse_use_mtl(StringView line, AssetParseDebugTracker *tracker, AssetObjData *obj) {
    char mtl_name[64] = {0};
    StringView tok = sv_trim_left(sv_chop(&line, ' '));
    asset_debug_parser_tracker_cursor_advance(tracker, line.data - tok.data);
    sv_strcopy(tok, mtl_name);
    AssetObjUseMtl *mtl = tda_back(&obj->use_mtl);
    if (mtl) {
        mtl->count = tda_size(&obj->indices) - mtl->index_start;
    }
    AssetObjUseMtl new_use = {0};
    for (size_t i = 0; i < tda_size(&obj->materials); i++) {
        AssetMtl *mtl  = tda_get(&obj->materials, i);
        if (mtl && strcmp(mtl_name, mtl->name) == 0) {
            new_use.material = tda_index_of(&obj->materials, mtl);
            new_use.index_start = tda_size(&obj->indices);
        }
    }

    return RESULT_OK;
}

Result asset_obj_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetObjData *out) {
    Result r = RESULT_OK;

    Array(char) content = {0};
    TRY(read_file(filename, "r", &content));
    StringView file = { tda_data(&content), tda_size(&content) };

    StringView sv_filename = sv_from_str(filename);
    StringView path = sv_chop_last(&sv_filename, '/');
    sv_strcopy(path, out->path);

    AssetObjGroup group = {0};
    sv_strcopy(sv_filename, group.name);
    group.first_index = 0;
    tda_push(&out->groups, &group);

    while (file.len) {
        asset_debug_parser_tracker_line_advance(tracker, 1);
        StringView line   = sv_trim_left(sv_chop(&file, '\n'));
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
                TRY_GOTO(r, fail, asset_obj_parse_and_load_mtllib(line, tracker, out));
                break;
            case ASSET_OBJ_ENTRY_TYPE_USE_MTL:
                break;
            case ASSET_OBJ_ENTRY_TYPE_S:
                printf("TODO: Impl parsing for type %s\n", entry_types_nm[type]);
                break;
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
    {
        AssetObjUseMtl *back = tda_back(&out->use_mtl);
        if (back && back->index_start < tda_size(&out->indices) && back->count == 0) {
            back->count = tda_size(&out->indices) - back->index_start;
        }
        for (size_t i = 0; i < tda_size(&out->groups); i++) {
            AssetObjGroup *cur = tda_get(&out->groups, i);
            if (cur && cur->index_count == 0) {
                tda_remove(&out->groups, i);
            }
        }
        asset_obj_group_close(out);
    }
    return r;
}
