#include "asset.h"
#include "core/core.h"
#include "core/result.h"
#include <core/sv.h>
#include <core/math.h>

const StringView asset_mtl_entry_type_sv[] = {
#define X(name, tok) SV(tok),
    ASSET_MTL_ENTRY_LIST
#undef X
};

const char *asset_mtl_entry_type_nm[] = {
#define X(name, tok) #name
    ASSET_MTL_ENTRY_LIST
#undef X
};

const AssetMaterialEntryType asset_mtl_entry_type_ls[] = {
#define X(name, tok) name,
    ASSET_MTL_ENTRY_LIST
#undef X
};


Result asset_mtl_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetMaterial *out) {
    Array(char) content = {0};
    TRY(read_file(filename, &content));
    StringView file = { tda_data(&content), tda_size(&content) };
    Result r = RESULT_OK;
    while (file.len) {
        tracker->line++;
        StringView line   = sv_trim_left(sv_chop_by_delim(&file, '\n'));
        StringView token  = sv_trim_left(sv_chop_by_delim(&line, ' '));
        AssetMaterialEntryType type = asset_entry_type_get(token, asset_mtl_entry_type_sv, (int *)asset_mtl_entry_type_ls, ARRAY_LEN(asset_mtl_entry_type_sv));
        switch (type) {
            case ASSET_MTL_ENTRY_KA: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 3, &v));
                out->ambient = v.v3;
                break;
            }
            case ASSET_MTL_ENTRY_KD: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 3, &v));
                out->diffuse = v.v3;
                break;
            }
            case ASSET_MTL_ENTRY_KS: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 3, &v));
                out->specular = v.v3;
                break;
            }
            case ASSET_MTL_ENTRY_NS: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 1, &v));
                out->specular_exponent = v.f;
                break;
            }
            case ASSET_MTL_ENTRY_D: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 1, &v));
                out->dissolve = v.f;
                break;
            }
            case ASSET_MTL_ENTRY_TR: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 1, &v));
                out->transparent = v.f;
                break;
            }
            case ASSET_MTL_ENTRY_TF: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 3, &v));
                out->transmission_filter = v.v3;
                break;
            }
            case ASSET_MTL_ENTRY_Ni: {
                OutVec v;
                TRY_GOTO(r, fail, asset_parse_vector(line, tracker, 1, &v));
                out->optical_density = v.f;
                break;
            }
            case ASSET_MTL_ENTRY_COUNT: break;
            default:
                printf("Invalid token: ");
                sv_print(token);
                printf("\n");
        }
    }
fail:
    tda_destroy(&content);
    return r;
}
