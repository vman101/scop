#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 m;
} pc;

void main() {
    gl_Position = pc.m * vec4(inPosition, 1.0);
    fragColor = vec3(1.0);
}
