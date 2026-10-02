#include "asset.h"

void
asset_debug_parser_tracker_line_advance(AssetParseDebugTracker *tracker, int32_t adv) {
    if (tracker) {
        tracker->line += adv;
        tracker->cursor = 0;
    }
}

void
asset_debug_parser_tracker_cursor_advance(AssetParseDebugTracker *tracker, ptrdiff_t adv) {
    if (tracker) {
        tracker->cursor += adv;
    }
}

void asset_debug_print_vec3_arr(const char *title, Array(Vec3) *position) {
    printf("%s\n", title);
    for (size_t i = 0; i < tda_size(position); ++i) {
        Vec3 pos = *tda_at(position, i);
        printf("  %zu: %f %f %f\n", i, pos.x, pos.y, pos.z);
    }
}

void asset_debug_print_index_arr(const char *title, Array(AssetObjIndex) *indices) {
    printf("%s\n", title);
    for (size_t i = 0; i < tda_size(indices); ++i) {
        AssetObjIndex index = *tda_at(indices, i);
        printf("  %zu: v: %d t: %d n: %d\n", i, index.position, index.texcoord, index.normal);
    }
}

void asset_debug_print_vector3(const char *title, Vec3 vec) {
    printf("  %s: x=%f y=%f z=%f\n", title, vec.x, vec.y, vec.z);
}

void asset_debug_print_float(const char *title, float f) {
    printf("  %s: %f\n", title, f);
}

void asset_debug_print_uint32_t(const char *title, uint32_t u) {
    printf("  %s: %u\n", title, u);
}

void asset_debug_print_mtl(AssetMtl *mtl) {
    printf("mtl %s\n", mtl->name);
    asset_debug_print_vector3("Ka", mtl->ambient);
    asset_debug_print_vector3("Kd", mtl->diffuse);
    asset_debug_print_vector3("Ks", mtl->specular);
    asset_debug_print_float("Ns", mtl->specular_exponent);
    asset_debug_print_float("d", mtl->dissolve);
    asset_debug_print_float("Tr", mtl->transparent);
    asset_debug_print_vector3("Tf", mtl->transmission_filter);
    asset_debug_print_float("Ni", mtl->optical_density);
    asset_debug_print_uint32_t("illum", mtl->illum);
}

void asset_debug_print_invalid_token(const char *filename, AssetParseDebugTracker *tracker, StringView token, int type, const char**entry_names) {
    printf("Invalid token: ");
    sv_print(token);
    printf("\t-> %s \n", entry_names[type]);
    if (tracker) {
        printf("  at %s\t%u:%u\n", filename, tracker->line, tracker->cursor);
    }
}
