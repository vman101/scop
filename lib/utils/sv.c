#include "sv.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool sv_eq(StringView a, StringView b) {
    return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

StringView sv_trim_left(StringView s) {
    while (s.len && isspace((unsigned char)*s.data)) {
        s.data++;
        s.len--;
    }
    return s;
}

StringView sv_chop_by_delim(StringView *s, char delim) {
    size_t i = 0;
    while (i < s->len && s->data[i] != delim) {
        i++;
    }
    StringView head = { s->data, i };
    size_t skip = i < s->len ? i + 1 : i;
    s->data += skip;
    s->len -= skip;
    return head;
}

void sv_print(StringView s) {
    printf("%.*s", (int)s.len, s.data);
}

Result sv_to_float(StringView s, float *out) {
    char buf[64];
    if (s.len == 0 || s.len >= sizeof(buf)) {
        return RESULT_ERR_PARSE_FLOAT;
    }
    memcpy(buf, s.data, s.len);
    buf[s.len] = '\0';
    char *end;
    *out = strtof(buf, &end);
    return (end == (buf + s.len)) ? RESULT_OK : RESULT_ERR_PARSE_FLOAT;
}

Result sv_to_long(StringView s, int32_t *out) {
    char buf[64];
    if (s.len == 0 || s.len >= sizeof(buf)) {
        return false;
    }
    memcpy(buf, s.data, s.len);
    buf[s.len] = '\0';
    char *end;
    *out = (int32_t)strtol(buf, &end, 10);
    return (end == (buf + s.len)) ? RESULT_OK : RESULT_ERR_PARSE_FLOAT;
}

