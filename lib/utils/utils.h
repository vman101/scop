#include "core/result.h"
#include <stdint.h>
#include "da.h"
#include "array_types.h"

#define ARRAY_LEN(a) ((sizeof (a)) / (sizeof (*(a))))

void        *alloc(uint32_t size);
Result      read_file(const char *filename, const char *flags, Array(char) *da);
int32_t     clamp(int32_t n, int32_t min, int32_t max);
uint32_t    uclamp(uint32_t n, uint32_t min, uint32_t max);
