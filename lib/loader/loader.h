#pragma once
#include "interface/mage_gfx.h"
#include "model/model.h"
#include "utils/result_tools.h"

#define LOADER_API

LOADER_API Result loader_model_load(GfxDevice dev, const char *filepath, Model *out);
