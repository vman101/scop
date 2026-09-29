#pragma once
#include <stdio.h> // IWYU pragma: export

#define RESULT_LIST(X) \
    X(RESULT_OK) \
    X(RESULT_ERR_ALLOC) \
    X(RESULT_ERR_VULKAN) \
    X(RESULT_ERR_FOPEN) \
    X(RESULT_ERR_GLFW) \
    X(RESULT_OUT_OF_SPACE) \
    X(RESULT_ERR_TODO) \
    X(RESULT_ERR_PARSE_FLOAT) \
    X(RESULT_ERR_PARSE_EXPECT)

#define RESULT_RANGE_CORE       0
#define RESULT_RANGE_GFX        1000
#define RESULT_RANGE_PLATFORM   2000
#define RESULT_RANGE_APP        3000

typedef enum {
#define X(name) name,
    RESULT_LIST(X)
#undef X
    RESULT_COUNT,
} Result;
