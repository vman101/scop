#pragma once

#include <interface/mage_result.h>
#include <stdint.h>
#include "array.h"

#define ARRAY_LEN(a) ((sizeof (a)) / (sizeof (*(a))))

typedef struct {
    uint32_t    start;
    uint32_t    count;
} Range;

DECLARE_ARRAY(Range);

void        *alloc(uint32_t size);
Result      read_file(const char *filename, const char *flags, Array(uint8_t) *da);
int32_t     clamp(int32_t n, int32_t min, int32_t max);
uint32_t    uclamp(uint32_t n, uint32_t min, uint32_t max);
