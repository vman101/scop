#pragma once

#include "interface/mage_result.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    ALLOCATOR_TYPE_LINEAR,
} AllocatorType;

typedef struct {
    AllocatorType type;
} AllocatorHeader;

typedef AllocatorHeader Allocator;

typedef struct {
    AllocatorHeader head;
    uint64_t        offset;
    uint64_t        size;
} LinearAllocator;

#define allocate(a, size, align, out) allocate_impl((Allocator *)(a), size, align, out)

Result linear_alloc(LinearAllocator *alloc, uint64_t size, uint64_t align, uint64_t *out);
void linear_init(LinearAllocator *alloc, size_t size);
size_t alloc_align(size_t size, uint32_t align);
Result allocate_impl(Allocator *alloc, uint64_t size, uint64_t align, uint64_t *out);

#ifdef ALLOCATORS_IMPLEMENTATION

size_t alloc_align(size_t size, uint32_t align) {
    return (size + align - 1) & ~(align - 1);
}

Result allocate_impl(Allocator *alloc, uint64_t size, uint64_t align, uint64_t *out) {
    switch (alloc->type) {
        case ALLOCATOR_TYPE_LINEAR: {
            LinearAllocator *la = (LinearAllocator *)alloc;
            linear_alloc(la, size, align, out);
            break ;
        }
    }
    return RESULT_OK;
}

void linear_init(LinearAllocator *alloc, size_t size) {
    *alloc = ((LinearAllocator) {
        .head = { .type = ALLOCATOR_TYPE_LINEAR },
        .offset = 0,
        .size = size,
    });
}

Result linear_alloc(LinearAllocator *alloc, uint64_t size, uint64_t align, uint64_t *out) {
    uint64_t offset = alloc_align(alloc->offset, align);
    if (offset + size > alloc->size) {
        return RESULT_OUT_OF_SPACE;
    }
    *out = offset;
    alloc->offset = offset + size;
    return RESULT_OK;
}

Result linear_dealloc(LinearAllocator *alloc) {
    (void)alloc;
    return RESULT_OK;
}

Result linear_realloc(LinearAllocator *alloc) {
    (void)alloc;
    return RESULT_OK;
}

Result linear_reset(LinearAllocator *alloc) {
    alloc->offset = 0;
    return RESULT_OK;
}

#endif
