#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 0) out float outDistance;

layout (set = 0, binding = 0) uniform A {
	mat4 projection;
    mat4 view;
    mat4 model;
} transform;

layout (push_constant) uniform B {
    vec4 position;
    vec4 color;
    float time;
} info;

void main() {
    vec4 position = vec4(inPosition + info.position.xyz, 1.0);
    outDistance = position.z;
    gl_Position = transform.projection * transform.view * position;
}
