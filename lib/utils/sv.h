#pragma once
#include <interface/mage_result.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char  *data;
    size_t      len;
} StringView;

#define SV(data) (StringView) { (data), (sizeof(data) - 1) }
#define sv_empty(s) ((s)->len == 0)

bool       sv_eq(StringView a, StringView b);
void       sv_print(StringView s);

StringView sv_from_str(const char *str);

StringView sv_trim_left(StringView s);
StringView sv_trim_right(StringView s);
StringView sv_trim(StringView s);

StringView sv_chop(StringView *s, char delim);
StringView sv_chop_last(StringView *s, char delim);

Result     sv_to_float(StringView s, float *out);
Result     sv_to_long(StringView s, int32_t *out);
void       sv_strcopy(StringView s, char *out);
