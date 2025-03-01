#version 450

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec4 v_color;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 color;

layout(set = 0, binding = 0) uniform Transform {
    mat4x4 transform;
} ubo;

void main() {
    vec4 pos = vec4(v_position, 0.0, 1.0); // Extend to vec3 for affine transformation
    gl_Position = vec4((ubo.transform * pos).xy, 0.0, 1.0);
    uv = v_uv;
    color = v_color;
}
