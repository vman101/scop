#version 460

struct Material {
    vec4 base_color;
};

layout(std430, set = 0, binding = 0) readonly buffer Materials {
    Material materials[];
};

layout(std430, set = 0, binding = 1) readonly buffer Texture {
    uint width;
    uint height;
    uint pixels[];
} tex;

layout(push_constant) uniform Push {
    layout(offset = 128) uint  material_index;
    layout(offset = 132) float tex_mix;
} pc;

layout(location = 0) in  vec3 v_normal;
layout(location = 1) in  vec3 v_world_pos;
layout(location = 2) in  vec2 v_uv;
layout(location = 3) in  flat uint face_id;
layout(location = 0) out vec4 outColor;

const vec3 LIGHT_DIR     = normalize(vec3(0.4, 1.0, 0.6));
const vec3 LIGHT_COLOR   = vec3(1.0);
const vec3 AMBIENT_LIGHT = vec3(0.15);
const vec3 CAMERA_POS    = vec3(0.0, 0.0, 8.0);

vec4 sample_tex(vec2 uv) {
    vec2 f = fract(uv);
    uvec2 p = min(
        uvec2(f * vec2(tex.width, tex.height)),
        uvec2(tex.width - 1, tex.height - 1)
    );
    return unpackUnorm4x8(tex.pixels[p.y * tex.width + p.x]);
}

void main() {
    Material m = materials[pc.material_index];

    vec3 N = normalize(v_normal);
    vec3 L = LIGHT_DIR;
    vec3 V = normalize(CAMERA_POS - v_world_pos);
    vec3 H = normalize(L + V);

    float ndl   = max(dot(N, L), 0.0);
    float g = 0.3 + 0.5 * fract(float(face_id) * 0.618);
    vec3 face_color = vec3(g);

    vec3 tex_color = m.base_color.rgb;
    vec3 albedo    = mix(face_color, tex_color, pc.tex_mix);
    vec3 color     = albedo * (AMBIENT_LIGHT + ndl * LIGHT_COLOR);

    // if (m.illum >= 2u && ndl > 0.0) {
    //     float shininess = m.specular_exponent > 0.0 ? m.specular_exponent : 32.0;
    //     color += m.specular * pow(max(dot(N, H), 0.0), shininess) * LIGHT_COLOR;
    // }

    outColor = vec4(color, 1.0);
}
