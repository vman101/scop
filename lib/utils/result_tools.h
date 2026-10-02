#include <interface/mage_result.h>

[[maybe_unused]] static const char *result_str(Result r) {
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
