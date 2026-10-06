#include "asset.h"
#include "interface/mage_result.h"
#include "utils/array.h"
#include "utils/utils.h"
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t  pos;
    size_t  size;
} Cursor;

static void
skip_ws_comments(Cursor *c) {
    while (c->pos < c->size) {
        uint8_t ch = c->data[c->pos];
        if (ch == '#') {
            while (c->pos < c->size && c->data[c->pos] != '\n') {
                c->pos++;
            }
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            c->pos++;
        } else {
            break ;
        }
    }
}

static bool
read_uint(Cursor *c, uint32_t *out) {
    skip_ws_comments(c);
    if (c->pos >= c->size || c->data[c->pos] < '0' || c->data[c->pos] > '9') {
        return false;
    }
    uint32_t v = 0;
    while (c->pos < c->size && c->data[c->pos] >= '0' && c->data[c->pos] <= '9') {
        if (v > (UINT32_MAX - 9) / 10) return false;
        v = (v * 10) + (c->data[c->pos++] - '0');
    }
    *out = v;

    return true;
}

Result
asset_image_parse_ppm(Array(uint8_t) *content, AssetImage *out) {
    Cursor c = {0};
    c.data = tda_data(content);
    c.size = tda_size(content);
    if (tda_size(content) < 2 || tda_data(content)[0] != 'P' || tda_data(content)[1] != '6') {
        return RESULT_ERR_TODO;
    }
    c.pos = 2;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t maxval = 0;
    if (!read_uint(&c, &width) || !read_uint(&c, &height) || !read_uint(&c, &maxval)) {
        return RESULT_ERR_TODO;
    }
    if (c.pos >= c.size) {
        return RESULT_ERR_TODO;
    }
    c.pos++; // skip one ws

    size_t count = (size_t)width * height;
    if (c.size - c.pos < (count * 3)) return RESULT_ERR_TODO;
    uint8_t *px = alloc(count * 4);
    if (!px) {
        return RESULT_ERR_ALLOC;
    }

    const uint8_t *src = c.data + c.pos;
    for (size_t i = 0; i < count; i++) {
        px[(i*4)+0] = src[(i*3)+0];
        px[(i*4)+1] = src[(i*3)+1];
        px[(i*4)+2] = src[(i*3)+2];
        px[(i*4)+0] = 255;
    }

    *out = (AssetImage){ width, height, (uint32_t *)px };
    return RESULT_OK;
}
