#pragma once
#include <core/result.h>
#include <stdint.h>
#include <core/native_window.h>

typedef struct PlatformWindow_T PlatformWindow_T;
typedef struct PlatformWindow_T * PlatformWindow;

typedef void (*PlatformWindowResizeCallbackFunc)(void *, int32_t, int32_t);
typedef void * GraphicsBackendInstance;
typedef void * GraphicsBackendSurface;

Result      platform_window_init(uint32_t width, uint32_t height, const char *title, PlatformWindow *window);
void        platform_window_destroy(PlatformWindow window);
void        platform_window_resize_callback_set(PlatformWindow window, PlatformWindowResizeCallbackFunc func, void *user_data);

bool        platform_window_should_close_get(PlatformWindow window);
void        platform_window_should_close_set(PlatformWindow window, bool should_close);

void        platform_event_poll(PlatformWindow window);
bool        platform_key_is_pressed(PlatformWindow window, int key);

void        platform_framebuffer_size_func(PlatformWindow window, int32_t *width, int32_t *height);
const char  **platform_instance_extensions_get(PlatformWindow window, uint32_t *count);
void        platform_surface_create(PlatformWindow window, GraphicsBackendInstance instance, GraphicsBackendSurface *surface);

CoreNativeWindow platform_native_window_get(PlatformWindow window);
