#pragma once

#include <utils/array_types.h>
#include <stdint.h>
#include "stdbool.h"
#include <interface/mage_math.h>
#include <utils/sv.h>
#include "utils/da.h"
#include "limits.h"

#define ASSET_MTL
#define ASSET_OBJ
#define ASSET_IMG
#define ASSET_UTL
#define ASSET_DBG


typedef struct AssetMtl       AssetMtl;
typedef struct AssetMtlLib    AssetMtlLib;
typedef struct Material       Material;
typedef struct AssetObjMtlLib AssetObjMtlLib;
typedef struct AssetObjData   AssetObjData;
typedef struct AssetObjIndex  AssetObjIndex;
typedef struct AssetObjUseMtl AssetObjUseMtl;
typedef struct AssetObjGroup  AssetObjGroup;
typedef struct AssetImage     AssetImage;

struct AssetImage {
    uint32_t w;
    uint32_t h;
    uint32_t *px;
};

DECLARE_ARRAY(Vec2);
DECLARE_ARRAY(Vec3);
DECLARE_ARRAY(AssetObjIndex);
DECLARE_ARRAY(AssetObjUseMtl);
DECLARE_ARRAY(AssetObjGroup);
DECLARE_ARRAY(AssetMtl);
DECLARE_ARRAY(Material);
DECLARE_ARRAY(AssetObjMtlLib);
DECLARE_ARRAY(AssetMtlLib);

#define ASSET_OBJ_NO_VALUE (INT_MAX)
#define ASSET_DEFAULT_MATERIAL_INDEX 0

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
    X(ASSET_MTL_ENTRY_NEWMTL,       "newmtl") \
    X(ASSET_MTL_ENTRY_KA,           "Ka") \
    X(ASSET_MTL_ENTRY_KD,           "Kd") \
    X(ASSET_MTL_ENTRY_KS,           "Ks") \
    X(ASSET_MTL_ENTRY_NS,           "Ns") \
    X(ASSET_MTL_ENTRY_D,            "d") \
    X(ASSET_MTL_ENTRY_TR,           "Tr") \
    X(ASSET_MTL_ENTRY_TF,           "Tf") \
    X(ASSET_MTL_ENTRY_NI,           "Ni") \
    X(ASSET_MTL_ENTRY_ILLUM,        "illum") \
    X(ASSET_MTL_ENTRY_COMMENT,      "#") \
    X(ASSET_MTL_ENTRY_TEX_KA,       "map_Ka") \
    X(ASSET_MTL_ENTRY_TEX_KD,       "map_Kd") \
    X(ASSET_MTL_ENTRY_TEX_KS,       "map_Ks") \
    X(ASSET_MTL_ENTRY_TEX_NS,       "map_Ns") \
    X(ASSET_MTL_ENTRY_TEX_D,        "map_d") \
    X(ASSET_MTL_ENTRY_TEX_BUMP,     "bump") \
    X(ASSET_MTL_ENTRY_TEX_MAP_BUMP, "map_bump") \
    X(ASSET_MTL_ENTRY_TEX_DISP,     "disp") \
    X(ASSET_MTL_ENTRY_TEX_DECAL,    "decal") \
    X(ASSET_MTL_ENTRY_TEX_REFL,     "refl") \
    X(ASSET_MTL_ENTRY_INVALID,      "$$_invalid")

typedef enum {
#define X(name, tok) name,
    ASSET_MTL_ENTRY_LIST
#undef X
    ASSET_MTL_ENTRY_COUNT,
} AssetMtlEntryType;

struct Material {
    Vec3     ambient;             float specular_exponent;
    Vec3     diffuse;             float dissolve;
    Vec3     specular;            float optical_density;
    Vec3     transmission_filter; float transparent;
    uint32_t illum;
    uint32_t _pad[3];
};

struct              AssetMtl {
    char            name[64];
    Material        mtl;
};

struct              AssetMtlLib  {
    char            name[64];
    Array(AssetMtl) mtls;
};

struct AssetObjMtlLib { char name[64]; };

typedef enum {
#define X(name, tok) name,
    ASSET_OBJ_ENTRY_LIST
#undef X
    ASSET_OBJ_RESULT_COUNT,
} AssetObjEntryType;

typedef struct { uint32_t cursor; uint32_t line; } AssetParseDebugTracker;

struct                    AssetObjGroup {
    char                  name[64];
    uint32_t              index_start;
    uint32_t              count;
};

struct                    AssetObjIndex {
    int32_t               position;
    int32_t               texcoord;
    int32_t               normal;
    int32_t               face_id;
};

struct                    AssetObjUseMtl {
    char                  name[64];
    uint32_t              index_start;
    uint32_t              count;
};

struct                    AssetObjData {
    Array(Vec3)           positions;
    Array(Vec2)           tex_coords;
    Array(Vec3)           normals;
    Array(AssetObjIndex)  indices;
    Array(AssetObjUseMtl) use_mtl;
    Array(AssetObjMtlLib) mtl_lib;
    Array(AssetObjGroup)  groups;
    Array(AssetMtl)       materials;
};

ASSET_OBJ void     asset_obj_destroy(AssetObjData *obj);
ASSET_OBJ Result   asset_obj_file_parse(const Array(char) *content, AssetParseDebugTracker *tracker, AssetObjData *out);

ASSET_MTL void     asset_mtl_destroy(AssetMtlLib *lib);
ASSET_MTL Result   asset_mtl_file_parse(const Array(char) *content, AssetParseDebugTracker *tracker, Array(AssetMtl) *out);
ASSET_MTL Material asset_mtl_material_default(void);

ASSET_IMG Result   asset_image_parse_ppm(Array(uint8_t) *content, AssetImage *out);

ASSET_UTL int      asset_entry_type_get(StringView s, const StringView entries_sv[], const int entries_ls[], size_t len);
ASSET_UTL Result   asset_parse_uint32_t(StringView line, AssetParseDebugTracker *tracker, uint32_t *out);
ASSET_UTL Result   asset_parse_string(StringView line, AssetParseDebugTracker *tracker, char out[64]);
ASSET_UTL Result   asset_parse_float(StringView line, AssetParseDebugTracker *tracker, float *out);
ASSET_UTL Result   asset_parse_vec2(StringView line, AssetParseDebugTracker *tracker, Vec2 *vec);
ASSET_UTL Result   asset_parse_vec3(StringView line, AssetParseDebugTracker *tracker, Vec3 *vec);
ASSET_UTL Result   asset_parse_vec2_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec2) *vec);
ASSET_UTL Result   asset_parse_vec3_into_arr(StringView line, AssetParseDebugTracker *tracker, Array(Vec3) *vec);

ASSET_DBG void     asset_debug_print_index_arr(const char *title, Array(AssetObjIndex) *indices);
ASSET_DBG void     asset_debug_print_vec3_arr(const char *title, Array(Vec3) *position);
ASSET_DBG void     asset_debug_print_mtl(const char *name, Material *mtl);
ASSET_DBG void     asset_debug_print_obj_data(const char *name, AssetObjData *obj);
ASSET_DBG void     asset_debug_print_vector3(const char *title, Vec3 vec);
ASSET_DBG void     asset_debug_print_float(const char *title, float f);
ASSET_DBG void     asset_debug_print_invalid_token(const char *filename, AssetParseDebugTracker *tracker, StringView token, int type, const char**entry_names);
ASSET_DBG void     asset_debug_parser_tracker_line_advance(AssetParseDebugTracker *tracker, int32_t adv);
ASSET_DBG void     asset_debug_parser_tracker_cursor_advance(AssetParseDebugTracker *tracker, ptrdiff_t adv);

#undef ASSET_UTL
#undef ASSET_MTL
#undef ASSET_OBJ
#undef ASSET_DBG
#undef ASSET_IMG
