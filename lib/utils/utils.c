#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "interface/mage_result.h"
#include "array.h"

const char *result_str(Result r) {
    static const char *names[] = {
#define X(name) #name,
        RESULT_LIST(X)
#undef X
    };
    return (r >= 0 && r < RESULT_COUNT) ? names[r] : "RESULT_UNKNOWN";
}

void print_err_with_location(Result r, const char *call, const char *file, const int line) {
    fprintf(stderr, "  %s failed (%s)\n    at %s:%d\n", call, result_str(r), file, line);
}

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
read_file(const char *filename, const char *flags, Array(uint8_t) *da) {
    FILE *shader_source = fopen(filename, flags);
    char *buffer = NULL;

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
