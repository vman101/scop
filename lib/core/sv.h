#pragma once

#include <core/result.h>
#include <core/core.h>

typedef struct {
    const char  *data;
    size_t      len;
} StringView;

#define SV(data) (StringView) { (data), (sizeof(data)) }

bool sv_eq(StringView a, StringView b);
bool sv_empty(StringView s);
StringView sv_trim_left(StringView s);
StringView sv_chop_by_delim(StringView *s, char delim);
