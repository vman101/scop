#pragma once
#include "da.h"
#include <interface/mage_math.h>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define DECLARE_ARRAY(T) union Array_##T { DynamicArray da; T *type_tag; }
#define DECLARE_ARRAY_NAMED(Name, T) \
    union Array_##Name { DynamicArray da; T *type_tag; }

// NOLINTEND(bugprone-macro-parentheses)

#define Array(T) union Array_##T

DECLARE_ARRAY_NAMED(CharPtr, char *);
DECLARE_ARRAY(char);
DECLARE_ARRAY(uint8_t);
DECLARE_ARRAY(uint32_t);

DECLARE_ARRAY(Vec2);
DECLARE_ARRAY(Vec3);
