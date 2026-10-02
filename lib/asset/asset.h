#pragma once

#include <utils/array_types.h>
#include <stdint.h>
#include "core/defines.h"
#include "stdbool.h"
#include <core/math.h>
#include <utils/sv.h>

typedef struct AssetMtl       AssetMtl;
typedef struct AssetObjData   AssetObjData;
typedef struct AssetObjIndex  AssetObjIndex;
typedef struct AssetObjUseMtl AssetObjUseMtl;
typedef struct AssetObjGroup  AssetObjGroup;

DECLARE_ARRAY(Vec2);
DECLARE_ARRAY(Vec3);
DECLARE_ARRAY(AssetObjIndex);
DECLARE_ARRAY(AssetObjUseMtl);
DECLARE_ARRAY(AssetObjGroup);
DECLARE_ARRAY(AssetMtl);

#define ASSET_OBJ_ENTRY_LIST \
    X(ASSET_OBJ_ENTRY_TYPE_V,       "v") \
    X(ASSET_OBJ_ENTRY_TYPE_F,       "f") \
    X(ASSET_OBJ_ENTRY_TYPE_VT,      "vt") \
    X(ASSET_OBJ_ENTRY_TYPE_VN,      "vn") \
    X(ASSET_OBJ_ENTRY_TYPE_O,       "o") \
    X(ASSET_OBJ_ENTRY_TYPE_G,       "g") \
    X(ASSET_OBJ_ENTRY_TYPE_S,       "s") \
    X(ASSET_OBJ_ENTRY_TYPE_COMMENT, "#") \
    X(ASSET_OBJ_ENTRY_TYPE_MTL_LIB, "mtllib") \
    X(ASSET_OBJ_ENTRY_TYPE_USE_MTL, "usemtl") \
    X(ASSET_OBJ_ENTRY_TYPE_INVALID, "$$_invalid")

#define ASSET_MTL_ENTRY_LIST \
    X(ASSET_MTL_ENTRY_NEWMTL,    "newmtl") \
    X(ASSET_MTL_ENTRY_KA,      "Ka") \
    X(ASSET_MTL_ENTRY_KD,      "Kd") \
    X(ASSET_MTL_ENTRY_KS,      "Ks") \
    X(ASSET_MTL_ENTRY_NS,      "Ns") \
    X(ASSET_MTL_ENTRY_D,       "d") \
    X(ASSET_MTL_ENTRY_TR,      "Tr") \
    X(ASSET_MTL_ENTRY_TF,      "Tf") \
    X(ASSET_MTL_ENTRY_NI,      "Ni") \
    X(ASSET_MTL_ENTRY_ILLUM,   "illum") \
    X(ASSET_MTL_ENTRY_COMMENT, "#") \
    X(ASSET_MTL_ENTRY_INVALID, "$$_invalid")

typedef enum {
#define X(name, tok) name,
    ASSET_MTL_ENTRY_LIST
#undef X
    ASSET_MTL_ENTRY_COUNT,
} AssetMtlEntryType;

struct AssetMtl {
    char        name[64];
    Vec3        ambient;
    Vec3        diffuse;
    Vec3        specular;
    float       specular_exponent;
    float       dissolve;
    float       transparent;
    Vec3        transmission_filter;
    float       optical_density;
    uint32_t    illum;
};

typedef enum {
#define X(name, tok) name,
    ASSET_OBJ_ENTRY_LIST
#undef X
    ASSET_OBJ_RESULT_COUNT,
} AssetObjEntryType;

typedef struct { uint32_t cursor; uint32_t line; } AssetParseDebugTracker;

struct                    AssetObjGroup {
    char                  name[64];
    uint32_t              first_index;
    uint32_t              index_count;
};

struct                    AssetObjIndex {
    int32_t               position;
    int32_t               texcoord;
    int32_t               normal;
};

struct                    AssetObjUseMtl {
    uint32_t              material;
    uint32_t              index_start;
    uint32_t              count;
};

struct                    AssetObjData {
    char                  path[PATH_MAX];
    Array(Vec3)           positions;
    Array(Vec2)           tex_coords;
    Array(Vec3)           normals;
    Array(AssetObjIndex)  indices;
    Array(AssetObjUseMtl) use_mtl;
    Array(AssetObjGroup)  groups;
    Array(AssetMtl)       materials;
};

typedef union { Vec3 v3; Vec2 v2; float f; } OutVec;

Result          asset_obj_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetObjData *out);
Result          asset_mtl_file_load(const char *filename, AssetParseDebugTracker *tracker, Array(AssetMtl) *out);
int             asset_entry_type_get(StringView s, const StringView entries_sv[], const int entries_ls[], size_t len);

Result          asset_parse_uint32_t(StringView line, AssetParseDebugTracker *tracker, uint32_t *out);
Result          asset_parse_string(StringView line, AssetParseDebugTracker *tracker, char out[64]);
Result          asset_parse_float(StringView line, AssetParseDebugTracker *tracker, float *out);
Result          asset_parse_vec2(StringView line, AssetParseDebugTracker *tracker, Vec2 *vec);
Result          asset_parse_vec3(StringView line, AssetParseDebugTracker *tracker, Vec3 *vec);
Result          asset_parse_vec2_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec2) *vec);
Result          asset_parse_vec3_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec3) *vec);

void            asset_debug_print_index_arr(const char *title, Array(AssetObjIndex) *indices);
void            asset_debug_print_vec3_arr(const char *title, Array(Vec3) *position);
void            asset_debug_print_mtl(AssetMtl *mtl);

void            asset_debug_print_vector3(const char *title, Vec3 vec);
void            asset_debug_print_float(const char *title, float f);
void            asset_debug_print_invalid_token(const char *filename, AssetParseDebugTracker *tracker, StringView token, int type, const char**entry_names);
