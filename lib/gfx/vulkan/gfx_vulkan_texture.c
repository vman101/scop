#include "interface/mage_gfx.h"
#include "gfx_vulkan_internal.h"
#include "utils/utils.h"
#include "utils/result_tools.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Result
gfx_texture_create(GfxDevice dev, GfxTextureDesc *desc, GfxTexture *out) {
    Result     r     = RESULT_OK;
    GfxTexture tex   = alloc(sizeof(*tex));
    GfxBuffer  buf   = {0};

    size_t   count     = (size_t)desc->width * desc->height;
    size_t   px_bytes  = count * 4;   // RGBA8
    size_t   hdr_bytes = 2 * sizeof(uint32_t);
    uint32_t *data     = alloc(hdr_bytes + px_bytes);
    if (!data) return RESULT_ERR_ALLOC;

    data[0] = desc->width;
    data[1] = desc->height;
    memcpy(data + 2, desc->data, px_bytes);

    GfxBufferDesc buf_info = {0};
    buf_info.count         = count;
    buf_info.member_size   = 4;
    buf_info.mem           = GFX_MEMORY_UPLOAD;
    buf_info.usage         = GFX_BUFFER_STORAGE;
    buf_info.data          = data;

    TRY_GOTO(r, cleanup, gfx_buffer_create(dev, &buf_info, &buf));
    gfx_global_buffer_set(dev, GFX_GLOBAL_SLOT_TEXTURES, buf);

    MOVE(out, tex);
cleanup:
    free(tex);
    free(data);
    return r;
}

void
gfx_texture_destroy(GfxDevice dev, GfxTexture tex) {
    gfx_buffer_destroy(dev, tex->buf);
    free(tex);
}
