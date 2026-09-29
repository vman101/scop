#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "result.h"
#include <stdbool.h>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define DECLARE_ARRAY(T) union Array_##T { DynamicArray da; T *type_tag; }
#define DECLARE_ARRAY_NAMED(Name, T) \
    union Array_##Name { DynamicArray da; T *type_tag; }

// NOLINTEND(bugprone-macro-parentheses)

#define Array(T) union Array_##T

#define tda_create(a, cap) \
    da_create(&(a)->da, sizeof(*(a)->type_tag), (cap))

#define tda_push(a, mem) \
    da_push(&(a)->da, mem)

#define tda_at(a, i) \
    ((typeof((a)->type_tag))da_get_mem(&(a)->da, (i)))

#define tda_size(a) ((a)->da.size)
#define tda_data(a) (((typeof((a)->type_tag))(a)->da.data ))
#define tda_from(a, b, cap) \
    da_create_from(&(a)->da, sizeof(*(a)->type_tag), (cap), b)
#define tda_destroy(a) \
    da_destroy(&(a)->da)
#define tda_type(a) typeof(*(a)->type_tag)

typedef struct {
    void        *data;
    size_t      member_size;
    size_t      cap;
    size_t      size;
} DynamicArray;

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

DECLARE_ARRAY(char);

void *alloc(uint32_t size);
Result read_file(const char *filename, Array(char) *da);
int32_t clamp(int32_t n, int32_t min, int32_t max);
uint32_t uclamp(uint32_t n, uint32_t min, uint32_t max);

void da_set(DynamicArray *da, size_t index, void *mem);
Result da_create(DynamicArray *da, size_t member_size, size_t cap);
void da_create_from(DynamicArray *da, size_t member_size, size_t mem_size, void *mem);
Result da_push(DynamicArray *da, const void *elem);
bool da_pop(DynamicArray *da, void *out);
void *da_get(DynamicArray *da, size_t index);
void da_destroy(DynamicArray *da);
void da_remove_index(DynamicArray *da, size_t index);
void *da_get_mem(DynamicArray *da, size_t index);
