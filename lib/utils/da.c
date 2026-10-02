#include <string.h>
#include <stdlib.h>
#include "da.h"

Result da_create(DynamicArray *da, size_t member_size, size_t cap) {
    *da = (DynamicArray){ .member_size = member_size, .cap = cap };
    if (cap == 0) {
        return RESULT_OK;
    }
    da->data = calloc(cap, member_size);
    if (!da->data) {
        da->cap = 0;
        return RESULT_ERR_ALLOC;
    }
    return RESULT_OK;
}

void da_create_from(DynamicArray *da, size_t member_size, size_t mem_size, void *mem) {
    *da = (DynamicArray){0};
    da->size = mem_size;
    da->cap = mem_size;
    da->member_size = member_size;
    da->data = mem;
}

static Result da_resize(DynamicArray *da, uint32_t factor) {
    size_t new_cap = da->cap ? da->cap * factor : 8;
    if (new_cap > SIZE_MAX / da->member_size) {
        return RESULT_ERR_ALLOC;
    }
    uint8_t *new_data = realloc(da->data, new_cap * da->member_size);
    if (!new_data) {
        return RESULT_ERR_ALLOC;
    }
    da->data = new_data;
    da->cap = new_cap;
    return RESULT_OK;
}

Result da_push(DynamicArray *da, size_t member_size, const void *elem) {
    if (da->size == da->cap) {
        if (da->member_size == 0 || da->data == NULL) {
            da->member_size = member_size;
        }
        size_t new_cap = da->cap ? da->cap * 2 : 8;
        if (new_cap > SIZE_MAX / da->member_size) {
            return RESULT_ERR_ALLOC;
        }
        uint8_t *new_data = realloc(da->data, new_cap * da->member_size);
        if (!new_data) {
            return RESULT_ERR_ALLOC;
        }
        da->data = new_data;
        da->cap = new_cap;
    }
    memcpy(da->data + (da->size * da->member_size), elem, da->member_size);
    da->size++;
    return RESULT_OK;
}

bool da_pop(DynamicArray *da, void *out) {   // out may be NULL
    if (da->size == 0) {
        return false;
    }
    da->size--;
    if (out) {
        memcpy(out, da->data + (da->size * da->member_size), da->member_size);
    }
    return true;
}

void da_remove_index(DynamicArray *da, size_t index) {
    if (index >= da->size) {
        return ;
    }

    void *dest = &da->data[index * da->member_size];
    void *src = dest + da->member_size;

    memmove(dest, src, da->member_size);
    da->size--;
}

static
void *da_get_raw(DynamicArray *da, size_t index) {
    return da->data + (index * da->member_size);
}

void *da_get(DynamicArray *da, size_t index) {
    if (index >= da->size) {
        return NULL;
    }
    return da_get_raw(da, index);
}

void *da_get_mem(DynamicArray *da, size_t index) {
    if (index >= da->cap) {
        if (da_resize(da, 2) != RESULT_OK) {
            return NULL;
        }
    }
    return da_get_raw(da, index);
}

void da_set(DynamicArray *da, size_t index, void *mem) {
    if (index >= da->size) {
        return;
    }
    memmove(da->data + (index * da->member_size), mem, da->member_size);
}

void da_destroy(DynamicArray *da) {
    free(da->data);
    *da = (DynamicArray){0};
}

void da_dedupe(DynamicArray *da) {
    for (size_t i = 0; i < da->size; ++i) {
        size_t j = i + 1;
        while (j < da->size) {
            if (memcmp(da_get(da, i), da_get(da, j), da->member_size) == 0) {
                da_remove_index(da, j);
            } else {
                ++j;
            }
        }
    }
}
