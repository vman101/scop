#include "asset.h"
#include "utils/result_tools.h"
#include "utils/sv.h"
#include <string.h>
#include <utils/utils.h>

Result
asset_parse_vector(StringView line, AssetParseDebugTracker *tracker, uint32_t vec_len, OutVec *out) {
    float values[3]       = {0};
    const char *start_pos = line.data;
    size_t i              = 0;

    for (; i < vec_len && line.len; i++) {
        StringView tok  = sv_trim_left(sv_chop_by_delim(&line, ' '));
        tracker->cursor = tok.data - start_pos;
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

Result
asset_parse_uint32_t(StringView line, AssetParseDebugTracker *tracker, uint32_t *out) {
    StringView token = sv_chop_by_delim(&line, ' ');
    sv_print(line);
    tracker->cursor += line.data - token.data;
    TRY(sv_to_long(token, (int32_t *)out));
    return RESULT_OK;
}

Result
asset_parse_string(StringView line, AssetParseDebugTracker *tracker, char out[64]) {
    (void)tracker;
    StringView eol = sv_trim_left(sv_chop_by_delim(&line, ' '));
    memcpy(out, eol.data, eol.len);
    return RESULT_OK;
}
