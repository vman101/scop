#include "interface/platform.h"
#include <bits/types/struct_timeval.h>
#include <sys/time.h>
#include <time.h>

int64_t platform_time_ns(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return ((int64_t)tp.tv_sec * 1000000000LL) + (int64_t)tp.tv_nsec;
}
