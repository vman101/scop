#include "scop.h"
#include <string.h>

Result da_create(DynamicArray *da, size_t member_size, size_t cap) {
    *da = (DynamicArray){ .member_size = member_size, .cap = cap };
    if (cap == 0) {
        return RESULT_OK;
    }
    da->data = calloc(cap, member_size);   // checks overflow
    return da->data ? RESULT_OK : RESULT_ERR_ALLOC;
}

Result da_push(DynamicArray *da, const void *elem) {
    if (da->size == da->cap) {
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

bool da_pop(DynamicArray *da, void *out) {   // out may be nullptr
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

void *da_get(DynamicArray *da, size_t index) {
    if (index >= da->size) {
        return nullptr;
    }
    return da->data + (index * da->member_size);
}

void da_destroy(DynamicArray *da) {
    free(da->data);
    *da = (DynamicArray){0};
}
