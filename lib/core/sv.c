#include "sv.h"
#include <ctype.h>
#include <stdint.h>
#include <string.h>

bool sv_eq(StringView a, StringView b) {
    return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

StringView sv_trim_left(StringView s) {
    while (s.len && isspace((char)*s.data)) {
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

bool sv_empty(StringView s) {
    return s.len == 0;
}
