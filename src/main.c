#include "scop.h"

#define DEBUG_MODE

#ifdef DEBUG_MODE
    uint32_t debug_mode = 1;
#else
    uint32_t debug_mode = 0;
#endif

void *
alloc(uint32_t size) {
    return calloc(1, size);
}

int main(void) {
   App app = {0};
   Result r = app_init(&app);
   app_destroy(&app);
   return r == RESULT_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
