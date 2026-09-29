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

[[maybe_unused]] static inline const char *result_str(Result r) {
    static const char *names[] = {
#define X(name) #name,
        RESULT_LIST(X)
#undef X
    };
    return (r >= 0 && r < RESULT_COUNT) ? names[r] : "RESULT_UNKNOWN";
}

#define TRY_EXPECT(call, expect) do { \
    int32_t r_ = (int)(call); \
    if (r_ != (expect)) { \
        fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", #call, result_str(r_), __FILE__, __LINE__); \
        return (r_); \
    } } while (0)

#define VK_TRY(call) do { \
    int32_t r_ = (call); \
    if (r_ != VK_SUCCESS) { \
        fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", #call, result_str(r_), __FILE__, __LINE__); \
        return RESULT_ERR_VULKAN; \
    } } while (0)

#define TRY(call) do { \
    Result r_ = (call); \
    if (r_ != RESULT_OK) { \
        fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", #call, result_str(r_), __FILE__, __LINE__); \
        return (r_); \
    } } while (0)


#define TRY_GOTO(r, label, call) do { \
    (r) = (int)(call); \
    if ((r) != RESULT_OK) { \
        fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", #call, result_str(r), __FILE__, __LINE__); \
        goto label; \
    } } while (0)

#define VK_TRY_GOTO(r, label, call) do { \
    VkResult vr_ = (call); \
    if (vr_ != VK_SUCCESS) { \
        (r) = RESULT_ERR_VULKAN; \
        fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", #call, result_str(r), __FILE__, __LINE__); \
        goto label; \
    } } while (0)
