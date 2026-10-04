#version 460

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uint in_face_id;

layout(push_constant) uniform Push {
    mat4 mvp;
    mat4 model;
} pc;

layout(location = 0) out vec3 v_normal;
layout(location = 1) out vec3 v_world_pos;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out uint v_face_id;

void main() {
    v_normal    = mat3(pc.model) * in_normal;
    v_world_pos = (pc.model * vec4(in_pos, 1.0)).xyz;
    v_uv        = in_uv;
    v_face_id   = in_face_id;
    gl_Position = pc.mvp * vec4(in_pos, 1.0);
}
