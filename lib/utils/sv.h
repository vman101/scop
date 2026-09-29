#pragma once
#include <core/result.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char  *data;
    size_t      len;
} StringView;

#define SV(data) (StringView) { (data), (sizeof(data) - 1) }
#define sv_empty(s) ((s)->len == 0)

bool sv_eq(StringView a, StringView b);
StringView sv_trim_left(StringView s);
StringView sv_chop_by_delim(StringView *s, char delim);
void sv_print(StringView s);

Result sv_to_float(StringView s, float *out);
Result sv_to_long(StringView s, int32_t *out);

