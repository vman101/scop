#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "interface/mage_result.h"
#include <stdbool.h>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define DECLARE_ARRAY(T) union Array_##T { DynamicArray da; T *type_tag; }
#define DECLARE_ARRAY_NAMED(Name, T) \
    union Array_##Name { DynamicArray da; T *type_tag; }

// NOLINTEND(bugprone-macro-parentheses)

#define Array(T) union Array_##T

#define tda_sizeof(a) sizeof(*(a)->type_tag)
#define tda_push(a, mem) \
    da_push(&(a)->da, tda_sizeof((a)), (mem))

#define tda_dedupe(a) \
    da_dedupe(&(a)->da)

#define tda_at(a, i) \
    (&(a)->type_tag[(i)])

#define tda_at_safe(a, i) \
    (da_get_mem(&(a)->da, (i)), &(a)->type_tag[(i)])

#define tda_get(a, i) \
    (da_get(&(a)->da, (i)) ? &(a)->type_tag[(i)] : NULL)

#define tda_size(a) ((a)->da.size)
#define tda_data(a) ((a)->type_tag)
#define tda_from(a, b, cap) \
    da_create_from(&(a)->da, sizeof(*(a)->type_tag), (cap), (b))
#define tda_destroy(a) \
    da_destroy(&(a)->da)

#define tda_create(a, cap) \
    da_create(&(a)->da, tda_sizeof((a)), (cap))

#define tda_back(a) \
    tda_get((a), (tda_size((a)) - (tda_size((a)) > 0)))

#define tda_next(a) \
    tda_get((a), (a)->da.cursor++)

#define tda_ended(a) \
    ((a)->da.cursor == (a)->da.size)
#define tda_cursor(a) \
    ((a)->da.cursor)
#define tda_reset(a) \
    ((a)->da.cursor = 0)

#define tda_index_of(a, mem) \
    ((mem) >= tda_data(a) && (mem) < tda_data(a) + tda_size(a) \
        ? (ptrdiff_t)((mem) - tda_data(a)) : (ptrdiff_t)-1)


typedef struct {
    void        *data;
    size_t      member_size;
    size_t      cap;
    size_t      size;
    uint64_t    cursor;
} DynamicArray;
_Static_assert(offsetof(DynamicArray, data) == 0, "data must be first");
typedef char data_first_check[offsetof(DynamicArray, data) == 0 ? 1 : -1];


void    da_set(DynamicArray *da, size_t index, void *mem);
Result  da_create(DynamicArray *da, size_t member_size, size_t cap);
void    da_create_from(DynamicArray *da, size_t member_size, size_t mem_size, void *mem);
Result  da_push(DynamicArray *da, size_t member_size, const void *elem);
bool    da_pop(DynamicArray *da, void *out);
void    *da_get(DynamicArray *da, size_t index);
void    da_destroy(DynamicArray *da);
void    da_remove_index(DynamicArray *da, size_t index);
void    *da_get_mem(DynamicArray *da, size_t index);
void    da_dedupe(DynamicArray *da);
