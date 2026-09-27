#pragma once
#include <stdio.h> // IWYU pragma: export

typedef enum {
    RESULT_OK,
    RESULT_ERR_ALLOC,
    RESULT_ERR_VULKAN,
    RESULT_ERR_FOPEN,
    RESULT_ERR_GLFW,
    RESULT_COUNT,
} Result;

#define TRY_EXPECT(call, expect) do { \
    int32_t r_ = (int)(call); \
    if (r_ != (expect)) { \
        fprintf(stderr, "  %s failed (%d)\n    at %s:%d\n", #call, r_, __FILE__, __LINE__); \
        return (r_); \
    } } while (0)

#define VK_TRY(call) do { \
    int32_t r_ = (call); \
    if (r_ != VK_SUCCESS) { \
        fprintf(stderr, "  %s failed (%d)\n    at %s:%d\n", #call, r_, __FILE__, __LINE__); \
        return RESULT_ERR_VULKAN; \
    } } while (0)

#define TRY(call) do { \
    Result r_ = (call); \
    if (r_ != RESULT_OK) { \
        fprintf(stderr, "  %s failed (%d)\n    at %s:%d\n", #call, r_, __FILE__, __LINE__); \
        return (r_); \
    } } while (0)

