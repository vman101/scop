#pragma once
#include <interface/gfx.h>
#include <interface/platform.h>
#include <core/result.h>


typedef struct {
    PlatformWindow window;
} Sdk;

void sdk_init();

Result sdk_obj_load(Sdk *sdk, const char *filename);
