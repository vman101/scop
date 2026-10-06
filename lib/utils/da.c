#include <string.h>
#include <stdlib.h>
#include "da.h"
#include "result_tools.h"

Result da_create(DynamicArray *da, size_t member_size, size_t cap) {
    *da = (DynamicArray){ .cap = cap };
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

void da_create_from(DynamicArray *da, size_t mem_size, void *mem) {
    *da = (DynamicArray){0};
    da->size = mem_size;
    da->cap = mem_size;
    da->data = mem;
}

static Result da_resize(DynamicArray *da, uint32_t m_size, uint32_t new_size) {
    size_t new_cap = new_size;
    if (new_cap > SIZE_MAX / m_size) {
        return RESULT_ERR_ALLOC;
    }
    uint8_t *new_data = realloc(da->data, new_cap * m_size);
    if (!new_data) {
        return RESULT_ERR_ALLOC;
    }
    da->data = new_data;
    da->cap = new_cap;
    return RESULT_OK;
}

Result da_push(DynamicArray *da, size_t member_size, const void *elem) {
    if (da->size == da->cap) {
        size_t new_cap = da->cap ? da->cap * 2 : 8;
        if (new_cap > SIZE_MAX / member_size) {
            return RESULT_ERR_ALLOC;
        }
        uint8_t *new_data = realloc(da->data, new_cap * member_size);
        if (!new_data) {
            return RESULT_ERR_ALLOC;
        }
        da->data = new_data;
        da->cap = new_cap;
    }
    memcpy(da->data + (da->size * member_size), elem, member_size);
    da->size++;
    return RESULT_OK;
}

bool da_pop(DynamicArray *da, size_t member_size, void *out) {   // out may be NULL
    if (da->size == 0) {
        return false;
    }
    da->size--;
    if (out) {
        memcpy(out, da->data + (da->size * member_size), member_size);
    }
    return true;
}

void da_remove_index(DynamicArray *da, size_t member_size, size_t index) {
    if (index >= da->size) {
        return ;
    }

    void *dest = &da->data[index * member_size];
    void *src = dest + member_size;

    memmove(dest, src, member_size);
    da->size--;
}

static void *da_get_raw(const DynamicArray *da, size_t member_size, size_t index) {
    return da->data + (index * member_size);
}

void *da_get(const DynamicArray *da, size_t member_size, size_t index) {
    if (index >= da->size) {
        return NULL;
    }
    return da_get_raw(da, member_size, index);
}

Result da_append(DynamicArray *a, const DynamicArray *b, size_t member_size) {
    if (b->size == 0) return RESULT_OK;

    size_t new_size = a->size + b->size;
    if (a->cap < new_size) {
        size_t new_cap = a->cap ? a->cap * 2 : 8;
        if (new_cap < new_size) new_cap = new_size;
        TRY(da_resize(a, member_size, new_size));
    }
    memcpy((uint8_t *)a->data + (member_size * a->size), b->data, b->size * member_size);
    a->size = new_size;
    return RESULT_OK;
}

void *da_get_mem(DynamicArray *da, size_t member_size, size_t index) {
    if (index >= da->cap) {
        if (da_resize(da, member_size, da->cap ? da->cap * 2 : 8) != RESULT_OK) {
            return NULL;
        }
    }
    return da_get_raw(da, member_size, index);
}

void da_set(DynamicArray *da, size_t member_size, size_t index, void *mem) {
    if (index >= da->size) {
        return;
    }
    memmove((uint8_t *)da->data + (index * member_size), mem, member_size);
}

void da_destroy(DynamicArray *da) {
    free(da->data);
    *da = (DynamicArray){0};
}

void da_dedupe(DynamicArray *da, size_t member_size) {
    for (size_t i = 0; i < da->size; ++i) {
        size_t j = i + 1;
        while (j < da->size) {
            if (memcmp(da_get(da, member_size, i), da_get(da, member_size, j), member_size) == 0) {
                da_remove_index(da, member_size, j);
            } else {
                ++j;
            }
        }
    }
}
