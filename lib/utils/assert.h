#pragma once

#include <stdio.h> // IWYU pragma: export

#ifndef NDEBUG
  #define ASSERT(cond, msg) do { if (!(cond)) { \
      fprintf(stderr, "%s:%d: assert(%s) failed: %s\n", \
              __FILE__, __LINE__, #cond, msg); \
      __builtin_trap(); } } while (0)
#else
  #define ASSERT(cond, msg) ((void)0)
#endif


#define TEST(r, label, v) do {                                    \
    if ((v) == NULL) {                                            \
        print_err_with_location(r, __func__, __FILE__, __LINE__); \
        goto label;                                               \
    }                                                             \
} while (0)
