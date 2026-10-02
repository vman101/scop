#pragma once
#include <interface/mage_gfx.h>
#include <interface/mage_platform.h>
#include <interface/mage_result.h>


typedef struct {
    PlatformWindow window;
} Sdk;

void sdk_init();

Result sdk_obj_load(Sdk *sdk, const char *filename);
