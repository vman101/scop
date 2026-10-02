#include "asset.h"
#include "utils/result_tools.h"
#include "utils/sv.h"
#include <string.h>
#include <utils/utils.h>

static Result
asset_parse_vector(StringView line, AssetParseDebugTracker *tracker, uint32_t vec_len, OutVec *out) {
    float values[3]       = {0};
    const char *start_pos = line.data;
    size_t i              = 0;

    for (; i < vec_len && line.len; i++) {
        StringView tok  = sv_trim_left(sv_chop(&line, ' '));
        asset_debug_parser_tracker_cursor_advance(tracker, tok.data - start_pos);
        if (sv_empty(&tok)) { continue; }
        TRY(sv_to_float(tok, &values[i]));
    }
    if (i < (vec_len - 1)) {
        fprintf(stderr, "Error: expected %u fields, but got %zu\n", vec_len, i + 1);
        return RESULT_ERR_PARSE_EXPECT;
    }

    if (vec_len == 1) {
        *out = (OutVec){ .f = values[0] };
    } else if (vec_len == 2) {
        *out = (OutVec){ .v2 = (Vec2){ values[0], values[1] } };
    } else if (vec_len == 3) {
        *out = (OutVec){ .v3 = (Vec3){ values[0], values[1], values[2] } };
    }
    return RESULT_OK;
}

Result asset_parse_vec2(StringView line, AssetParseDebugTracker *tracker, Vec2 *vec) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 2, &v));
    *vec = v.v2;
    return RESULT_OK;
}

Result asset_parse_vec3(StringView line, AssetParseDebugTracker *tracker, Vec3 *vec) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 3, &v));
    *vec = v.v3;
    return RESULT_OK;
}

Result asset_parse_vec2_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec2) *vec) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 2, &v));
    tda_push(vec, &v.v2);
    return RESULT_OK;
}

Result asset_parse_vec3_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec3) *vec) {
    OutVec v;
    TRY(asset_parse_vector(line, tracker, 3, &v));
    tda_push(vec, &v.v3);
    return RESULT_OK;
}

Result
asset_parse_uint32_t(StringView line, AssetParseDebugTracker *tracker, uint32_t *out) {
    StringView token = sv_chop(&line, ' ');
    sv_print(line);
    asset_debug_parser_tracker_cursor_advance(tracker, line.data - token.data);
    TRY(sv_to_long(token, (int32_t *)out));
    return RESULT_OK;
}

Result
asset_parse_float(StringView line, AssetParseDebugTracker *tracker, float *out) {
    StringView token = sv_chop(&line, ' ');
    sv_print(line);
    asset_debug_parser_tracker_cursor_advance(tracker, line.data - token.data);
    TRY(sv_to_float(token, (float *)out));
    return RESULT_OK;
}

Result
asset_parse_string(StringView line, AssetParseDebugTracker *tracker, char out[64]) {
    (void)tracker;
    StringView eol = sv_trim_left(sv_chop(&line, ' '));
    memcpy(out, eol.data, eol.len);
    return RESULT_OK;
}
