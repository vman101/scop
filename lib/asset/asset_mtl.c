#include "asset.h"
#include "core/result.h"
#include <utils/utils.h>
#include <utils/result_tools.h>
#include <core/math.h>

const StringView asset_mtl_entry_type_sv[] = {
#define X(name, tok) SV(tok),
    ASSET_MTL_ENTRY_LIST
#undef X
};

const char *asset_mtl_entry_type_nm[] = {
#define X(name, tok) #name,
    ASSET_MTL_ENTRY_LIST
#undef X
};

const AssetMtlEntryType asset_mtl_entry_type_ls[] = {
#define X(name, tok) name,
    ASSET_MTL_ENTRY_LIST
#undef X
};

int asset_entry_type_get(StringView s, const StringView entries_sv[], const int entries_ls[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (sv_eq(s, entries_sv[i])) {
            return entries_ls[i];
        }
    }
    return entries_ls[len - 1];
}

Result asset_mtl_file_load(const char *filename, AssetParseDebugTracker *tracker, Array(AssetMtl) *out) {
    Array(char) content = {0};
    TRY(read_file(filename, "r", &content));
    StringView file     = { tda_data(&content), tda_size(&content) };
    Result r            = RESULT_OK;
    AssetMtl *target    = NULL;

    while (file.len) {
        tracker->cursor = 0;
        tracker->line++;
        StringView line  = sv_trim_left(sv_chop(&file, '\n'));
        if (sv_empty(&line) || line.data[0] == '#') { continue; }
        StringView token = sv_trim_left(sv_chop(&line, ' '));
        AssetMtlEntryType type = asset_entry_type_get(token, asset_mtl_entry_type_sv, (int *)asset_mtl_entry_type_ls, ARRAY_LEN(asset_mtl_entry_type_sv));
        if (type != ASSET_MTL_ENTRY_NEWMTL && target == NULL) {
            fprintf(stderr, "Error: got token type before defining a single newmtl");
            return RESULT_ERR_PARSE_EXPECT;
        }
        AssetMtl new = {0};
        switch (type) {
            case ASSET_MTL_ENTRY_KA:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->ambient));
                break;
            case ASSET_MTL_ENTRY_KD:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->diffuse));
                break;
            case ASSET_MTL_ENTRY_KS:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->specular));
                break;
            case ASSET_MTL_ENTRY_NS:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->specular_exponent));
                break;
            case ASSET_MTL_ENTRY_D:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->dissolve));
                break;
            case ASSET_MTL_ENTRY_TR:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->transparent));
                break;
            case ASSET_MTL_ENTRY_TF:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->transmission_filter));
                break;
            case ASSET_MTL_ENTRY_NI:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->optical_density));
                break;
            case ASSET_MTL_ENTRY_NEWMTL:
                TRY_GOTO(r, fail, tda_push(out, &new));
                target = tda_back(out);
                TRY_GOTO(r, fail, asset_parse_string(line, tracker, (char *)&target->name));
                break;
            case ASSET_MTL_ENTRY_ILLUM:
                TRY_GOTO(r, fail, asset_parse_uint32_t(line, tracker, &target->illum));
                break;
            case ASSET_MTL_ENTRY_COMMENT:
            case ASSET_MTL_ENTRY_COUNT:
                break;
            default:
            case ASSET_MTL_ENTRY_INVALID:
                asset_debug_print_invalid_token(filename, tracker, token, type, asset_mtl_entry_type_nm);
        }
    }
fail:
    tda_destroy(&content);
    return r;
}
