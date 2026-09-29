#pragma once

typedef enum {
    PLATFORM_WS_WIN32,
    PLATFORM_WS_X11,
    PLATFORM_WS_WAYLAND,
    PLATFORM_WS_COCOA,
} CoreWindowSystem;

typedef struct CoreNativeWindow {
    CoreWindowSystem type;
    union {
        struct { void *hinstance, *hwnd; } win32;
        struct { void *display; unsigned long window; } x11;
        struct { void *display, *surface; } wayland;
        struct { void *layer; } cocoa; // CAMetalLayer*
    };
} CoreNativeWindow;
