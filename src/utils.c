#include "scop.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

void *
alloc(uint32_t size) {
    return calloc(1, size);
}

Result
read_file(const char *filename, DynamicArray *da) {
    FILE *shader_source = fopen(filename, "rb");
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

    TRY(da_create_from(da, sizeof(*buffer), file_size, buffer));

    fclose(shader_source);

    return RESULT_OK;
}
