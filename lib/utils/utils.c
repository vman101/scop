#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "core/result.h"
#include "array_types.h"

int32_t clamp(int32_t n, int32_t min, int32_t max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

uint32_t uclamp(uint32_t n, uint32_t min, uint32_t max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}

void *
alloc(uint32_t size) {
    return calloc(1, size);
}

Result
read_file(const char *filename, const char *flags, Array(char) *da) {
    FILE *shader_source = fopen(filename, flags);
    char *buffer = nullptr;

    if (!shader_source) {
        fprintf(stderr, "Error: failed to open file '%s': %s\n", filename, strerror(errno));
        return RESULT_ERR_FOPEN;
    }

    fseek(shader_source, 0, SEEK_END);
    const size_t file_size = ftell(shader_source);
    buffer = alloc(file_size);
    if (!buffer) {
        return RESULT_ERR_ALLOC;
    }
    fseek(shader_source, 0, SEEK_SET);

    const size_t bytes_read = fread(buffer, 1, file_size, shader_source);
    if (bytes_read != file_size) {
        fprintf(stderr, "Error: read invalid amount of bytes: expect %zu got %zu\n", file_size, bytes_read);
    }

    tda_from(da, buffer, bytes_read);

    fclose(shader_source);

    return RESULT_OK;
}
