#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "interface/mage_result.h"
#include <stdbool.h>

#define tda_sizeof(a) sizeof(*(a)->type_tag)
#define tda_push(a, mem) \
    da_push(&(a)->da, tda_sizeof((a)), (mem))

#define tda_dedupe(a) \
    da_dedupe(&(a)->da, tda_sizeof((a)))

#define tda_at(a, i) \
    (&(a)->type_tag[(i)])

#define tda_at_safe(a, i) \
    (da_get_mem(&(a)->da, tda_sizeof((a)), (i)), &(a)->type_tag[(i)])

#define tda_get(a, i) \
    (da_get(&(a)->da, tda_sizeof((a)), (i)) ? &(a)->type_tag[(i)] : NULL)

#define tda_size(a) ((a)->da.size)
#define tda_data(a) ((a)->type_tag)
#define tda_from(a, b, cap) \
    da_create_from(&(a)->da, (cap), (b))
#define tda_destroy(a) \
    da_destroy(&(a)->da)

#define tda_create(a, cap) \
    da_create(&(a)->da, tda_sizeof((a)), (cap))

#define tda_back(a) \
    tda_get((a), (tda_size((a)) - (tda_size((a)) > 0)))

#define tda_index_of(a, mem) \
    ((mem) >= tda_data(a) && (mem) < tda_data(a) + tda_size(a) \
        ? (ptrdiff_t)((mem) - tda_data(a)) : (ptrdiff_t)-1)

#define tda_remove(a, idx) \
    da_remove_index(&(a)->da, tda_sizeof((a)), (idx))

#define tda_append(a, b) \
    da_append(&(a)->da, &(b)->da, tda_sizeof((a)))

typedef struct {
    void        *data;
    size_t      cap;
    size_t      size;
} DynamicArray;

_Static_assert(offsetof(DynamicArray, data) == 0, "data must be first");
typedef char data_first_check[offsetof(DynamicArray, data) == 0 ? 1 : -1];

void   da_set(DynamicArray *da, size_t member_size, size_t index, void *mem);
Result da_create(DynamicArray *da, size_t member_size, size_t cap);
void   da_create_from(DynamicArray *da, size_t mem_size, void *mem);
Result da_push(DynamicArray *da, size_t member_size, const void *elem);
bool   da_pop(DynamicArray *da, size_t member_size, void *out);
Result da_append(DynamicArray *a, const DynamicArray *b, size_t member_size);
void   *da_get(const DynamicArray *da, size_t member_size, size_t index);
void   da_destroy(DynamicArray *da);
void   da_remove_index(DynamicArray *da, size_t member_size, size_t index);
void   *da_get_mem(DynamicArray *da, size_t member_size, size_t index);
void   da_dedupe(DynamicArray *da, size_t member_size);
