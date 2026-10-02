#include "utils/da.h"
#if defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
    #define VK_USE_PLATFORM_METAL_EXT
#elif defined(__linux__)
    #define VK_USE_PLATFORM_XLIB_KHR
    #define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include <utils/utils.h>
#include <utils/result_tools.h>

#include <interface/mage_result.h>
#include <stdint.h>
#include <interface/mage_gfx.h>
#include "gfx_vulkan_internal.h"
#include <interface/mage_native_window.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *name;
    CoreWindowSystem ws; // which window system this extension serves
} SurfaceExt;
 
static const SurfaceExt k_platform_surface_exts[] = {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    { VK_KHR_WIN32_SURFACE_EXTENSION_NAME,   NATIVE_WS_WIN32   },
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    { VK_KHR_XLIB_SURFACE_EXTENSION_NAME,    PLATFORM_WS_X11     },
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    { VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, PLATFORM_WS_WAYLAND },
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    { VK_EXT_METAL_SURFACE_EXTENSION_NAME,   NATIVE_WS_COCOA   },
#endif
};
#define PLATFORM_SURFACE_EXT_COUNT (sizeof k_platform_surface_exts / sizeof k_platform_surface_exts[0])

static bool ext_available(const VkExtensionProperties *props, uint32_t n, const char *name) {
    for (uint32_t i = 0; i < n; i++) {
        if (strcmp(props[i].extensionName, name) == 0) {
            return true;
        }
    }
    return false;
}

// Appends VK_KHR_surface + every available platform surface extension to `out`.
// Writes a bitmask of usable window systems (1u << NativeWindowSystem) to
// `ws_mask`; store it on the device and check it at surface creation.
// Returns the number of extensions written, or 0 if presentation is unsupported.
Result gfx_platform_surface_instance_extensions_get(uint32_t *ws_mask, char ***extensions_out, uint32_t *count) {
    *ws_mask = 0;
    vkEnumerateInstanceExtensionProperties(NULL, count, NULL);
    VkExtensionProperties *props = alloc((*count) * sizeof *props);
    if (!props) {
        return RESULT_ERR_ALLOC;
    }
    vkEnumerateInstanceExtensionProperties(NULL, count, props);

    Array(CharPtr) out = {0};
    TRY(tda_create(&out, *count));

    Result res = RESULT_OK;
    if (ext_available(props, *count, VK_KHR_SURFACE_EXTENSION_NAME)) {
        const char *surface_ext = VK_KHR_SURFACE_EXTENSION_NAME;
        res = tda_push(&out, (void *)&surface_ext);
        for (uint32_t i = 0; res == RESULT_OK && i < PLATFORM_SURFACE_EXT_COUNT; i++) {
            if (ext_available(props, *count, k_platform_surface_exts[i].name)) {
                res = tda_push(&out, (void *)&k_platform_surface_exts[i].name);
                *ws_mask |= 1U << k_platform_surface_exts[i].ws;
            }
        }
    }

    free(props); // or your allocator's matching free
    *extensions_out = tda_data(&out);
    *count = tda_size(&out);
    if (res == RESULT_OK && *ws_mask == 0) {
        res = RESULT_ERR_VULKAN;
    }
    return res;
}
Result gfx_platform_surface_create(GfxDevice dev, uint32_t ws_mask, const CoreNativeWindow *w) {
    VkInstance inst = dev->ctx.instance;
    if (!(ws_mask & (1U << w->type))) {
        return RESULT_ERR_VULKAN;
    }

    switch (w->type) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    case NATIVE_WS_WIN32: {
        VkWin32SurfaceCreateInfoKHR ci = {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = (HINSTANCE)w->win32.hinstance,
            .hwnd      = (HWND)w->win32.hwnd,
        };
        VK_TRY(vkCreateWin32SurfaceKHR(inst, &ci, NULL, &dev->ctx.surface));
        return RESULT_OK;
    }
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    case PLATFORM_WS_X11: {
        VkXlibSurfaceCreateInfoKHR ci = {
            .sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .dpy    = (Display *)w->x11.display,
            .window = (Window)w->x11.window,
        };
        VK_TRY(vkCreateXlibSurfaceKHR(inst, &ci, NULL, &dev->ctx.surface));
        return RESULT_OK;
    }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    case PLATFORM_WS_WAYLAND: {
        VkWaylandSurfaceCreateInfoKHR ci = {
            .sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .display = (struct wl_display *)w->wayland.display,
            .surface = (struct wl_surface *)w->wayland.surface,
        };
        VK_TRY(vkCreateWaylandSurfaceKHR(inst, &ci, NULL, &dev->ctx.surface));
        return RESULT_OK;
    }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    case NATIVE_WS_COCOA: {
        VkMetalSurfaceCreateInfoEXT ci = {
            .sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
            .pLayer = (const CAMetalLayer *)w->cocoa.layer,
        };
        VK_TRY(vkCreateMetalSurfaceEXT(inst, &ci, NULL, &dev->ctx.surface));
        return RESULT_OK;
    }
#endif
    default:
        return RESULT_ERR_VULKAN;
    }
}
