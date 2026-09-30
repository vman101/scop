#pragma once

#include <utils/array_types.h>
#include <stdint.h>
#include "stdbool.h"
#include <core/math.h>
#include <utils/sv.h>

#define ASSET_OBJ_ENTRY_LIST \
    X(ASSET_OBJ_ENTRY_TYPE_V, "v") \
    X(ASSET_OBJ_ENTRY_TYPE_F, "f") \
    X(ASSET_OBJ_ENTRY_TYPE_VT, "vt") \
    X(ASSET_OBJ_ENTRY_TYPE_VN, "vn") \
    X(ASSET_OBJ_ENTRY_TYPE_O, "o") \
    X(ASSET_OBJ_ENTRY_TYPE_G, "g") \
    X(ASSET_OBJ_ENTRY_TYPE_S, "s") \
    X(ASSET_OBJ_ENTRY_TYPE_COMMENT, "#") \
    X(ASSET_OBJ_ENTRY_TYPE_MTL_LIB, "mtllib") \
    X(ASSET_OBJ_ENTRY_TYPE_USE_MTL, "usemtl") \
    X(ASSET_OBJ_ENTRY_TYPE_INVALID, "$$_invalid")

#define ASSET_MTL_ENTRY_LIST \
    X(ASSET_MTL_ENTRY_NAME, "newmtl") \
    X(ASSET_MTL_ENTRY_KA, "Ka") \
    X(ASSET_MTL_ENTRY_KD, "Kd") \
    X(ASSET_MTL_ENTRY_KS, "Ks") \
    X(ASSET_MTL_ENTRY_NS, "Ns") \
    X(ASSET_MTL_ENTRY_D, "d") \
    X(ASSET_MTL_ENTRY_TR, "Tr") \
    X(ASSET_MTL_ENTRY_TF, "Tf") \
    X(ASSET_MTL_ENTRY_Ni, "Ni") \
    X(ASSET_MTL_ENTRY_ILLUM, "illum") \
    X(ASSET_MTL_ENTRY_COMMENT, "#") \
    X(ASSET_MTL_ENTRY_INVALID, "$$_invalid")

typedef enum {
#define X(name, tok) name,
    ASSET_OBJ_ENTRY_LIST
#undef X
    ASSET_OBJ_RESULT_COUNT,
} AssetObjEntryType;

typedef struct { uint32_t cursor; uint32_t line; } AssetParseDebugTracker;

typedef struct {
    char        name[64];
    uint32_t    first_index;
    uint32_t    index_count;
} AssetObjGroup;

typedef struct {
    int32_t position;
    int32_t texcoord;
    int32_t normal;
} AssetObjIndex;

typedef struct {
    uint32_t material;
    uint32_t index_start;
    uint32_t count;
} AssetObjUseMtl;

DECLARE_ARRAY(Vec2);
DECLARE_ARRAY(Vec3);
DECLARE_ARRAY(AssetObjIndex);
DECLARE_ARRAY(AssetObjUseMtl);
DECLARE_ARRAY(AssetObjGroup);

typedef struct {
    Array(Vec3)             positions;
    Array(Vec2)             tex_coords;
    Array(Vec3)             normals;
    Array(AssetObjIndex)    indices;
    Array(AssetObjUseMtl)   materials;
    Array(AssetObjGroup)    groups;
} AssetObjData;

typedef enum {
#define X(name, tok) name,
    ASSET_MTL_ENTRY_LIST
#undef X
    ASSET_MTL_ENTRY_COUNT,
} AssetMaterialEntryType;

typedef struct {
    const char  *name;
    Vec3        ambient;
    Vec3        diffuse;
    Vec3        specular;
    float       specular_exponent;
    float       dissolve;
    float       transparent;
    Vec3        transmission_filter;
    float       optical_density;
    uint32_t    illum;
} AssetMaterial;

typedef union { Vec3 v3; Vec2 v2; float f; } OutVec;

Result          asset_obj_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetObjData *out);
AssetObjData    asset_obj_data_init(void);
int             asset_entry_type_get(StringView s, const StringView entries_sv[], const int entries_ls[], size_t len);
Result          asset_parse_vector(StringView line, AssetParseDebugTracker *tracker, uint32_t vec_len, OutVec *out);
Result          asset_parse_uint32_t(StringView line, AssetParseDebugTracker *tracker, uint32_t *out);
Result          asset_parse_string(StringView line, AssetParseDebugTracker *tracker, char out[64]);

Result          asset_mtl_file_load(const char *filename, AssetParseDebugTracker *tracker, AssetMaterial *out);

void            asset_obj_debug_print_index_arr(const char *title, Array(AssetObjIndex) *indices);
void            asset_obj_debug_print_vec3_arr(const char *title, Array(Vec3) *position);
void            asset_debug_mtl_print(AssetMaterial *mtl);

void            asset_debug_print_vector3(const char *title, Vec3 vec);
void            asset_debug_print_float(const char *title, float f);
void            asset_debug_print_invalid_token(const char *filename, AssetParseDebugTracker *tracker, StringView token, int type, const char**entry_names);
