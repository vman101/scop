#include "asset.h"
#include "interface/mage_result.h"
#include "utils/da.h"
#include <utils/utils.h>
#include <utils/result_tools.h>

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

void
asset_mtl_destroy(AssetMtlLib *lib) {
    if (!lib) { return ; }
    tda_destroy(&lib->mtls);
}

Material
asset_mtl_material_default(void) {
    Material m = {0};
    m.diffuse = vec3(0.8F, 0.8F, 0.8F);
    m.specular = vec3(0.8F, 0.8F, 0.8F);
    m.dissolve = 1;
    m.illum = 2;
    return m;
}

int asset_entry_type_get(StringView s, const StringView entries_sv[], const int entries_ls[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (sv_eq(s, entries_sv[i])) {
            return entries_ls[i];
        }
    }
    return entries_ls[len - 1];
}

Result
asset_mtl_file_parse(const Array(char) *content, AssetParseDebugTracker *tracker, Array(AssetMtl) *out) {
    StringView file     = { tda_data(content), tda_size(content) };
    Result r            = RESULT_OK;
    AssetMtl *target    = NULL;

    Array(AssetMtl) asset_mtl = {0};

    while (file.len) {
        asset_debug_parser_tracker_line_advance(tracker, 1);
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
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->mtl.ambient));
                break;
            case ASSET_MTL_ENTRY_KD:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->mtl.diffuse));
                break;
            case ASSET_MTL_ENTRY_KS:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->mtl.specular));
                break;
            case ASSET_MTL_ENTRY_NS:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->mtl.specular_exponent));
                break;
            case ASSET_MTL_ENTRY_D:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->mtl.dissolve));
                break;
            case ASSET_MTL_ENTRY_TR:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->mtl.transparent));
                break;
            case ASSET_MTL_ENTRY_TF:
                TRY_GOTO(r, fail, asset_parse_vec3(line, tracker, &target->mtl.transmission_filter));
                break;
            case ASSET_MTL_ENTRY_NI:
                TRY_GOTO(r, fail, asset_parse_float(line, tracker, &target->mtl.optical_density));
                break;
            case ASSET_MTL_ENTRY_NEWMTL:
                TRY_GOTO(r, fail, tda_push(&asset_mtl, &new));
                target = tda_back(&asset_mtl);
                TRY_GOTO(r, fail, asset_parse_string(line, tracker, (char *)&target->name));
                break;
            case ASSET_MTL_ENTRY_ILLUM:
                TRY_GOTO(r, fail, asset_parse_uint32_t(line, tracker, &target->mtl.illum));
                break;
            case ASSET_MTL_ENTRY_COMMENT:
            case ASSET_MTL_ENTRY_COUNT:
                break;
                printf("TODO: Impl parsing for type %s\n", asset_mtl_entry_type_nm[type]);
            default:
            case ASSET_MTL_ENTRY_INVALID:
                asset_debug_print_invalid_token(NULL, tracker, token, type, asset_mtl_entry_type_nm);
        }
    }
    *out = asset_mtl;
    asset_mtl = (Array(AssetMtl)){0};
fail:
    tda_destroy(&asset_mtl);
    return r;
}
