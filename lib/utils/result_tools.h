#include <interface/mage_result.h>
#include <stdio.h>

#define RESULT_TOOLS

#define MOVE(dst, src) do { *(dst) = (src); (src) = (__typeof__(src)){0}; } while (0)

RESULT_TOOLS [[maybe_unused]] static const char *result_str(Result r) {
    static const char *names[] = {
#define X(name) #name,
        RESULT_LIST(X)
#undef X
    };
    return (r >= 0 && r < RESULT_COUNT) ? names[r] : "RESULT_UNKNOWN";
}

RESULT_TOOLS static void print_err_with_location(Result r, const char *call, const char *file, const int line) {
    fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", call, result_str(r), file, line);
}

#define TRY_EXPECT(call, expect) do { \
    int32_t r_ = (int)(call); \
    if (r_ != (expect)) { \
        print_err_with_location(r_, #call, __FILE__, __LINE__); \
        return (r_); \
    } } while (0)

#define VK_TRY(call) do { \
    int32_t r_ = (call); \
    if (r_ != VK_SUCCESS) { \
        print_err_with_location(r_, #call, __FILE__, __LINE__); \
        return RESULT_ERR_VULKAN; \
    } } while (0)

#define TRY(call) do { \
    Result r_ = (call); \
    if (r_ != RESULT_OK) { \
        print_err_with_location(r_, #call, __FILE__, __LINE__); \
        return (r_); \
    } } while (0)

#define TRY_GOTO(r, label, call) do { \
    (r) = (int)(call); \
    if ((r) != RESULT_OK) { \
        print_err_with_location(r, #call, __FILE__, __LINE__); \
        goto label; \
    } } while (0)

#define VK_TRY_GOTO(r, label, call) do { \
    VkResult vr_ = (call); \
    if (vr_ != VK_SUCCESS) { \
        (r) = RESULT_ERR_VULKAN; \
        print_err_with_location(r, #call, __FILE__, __LINE__); \
        goto label; \
    } } while (0)

#undef RESULT_TOOLS
