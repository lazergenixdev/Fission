#version 450 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexcoord;
layout (location = 0) out vec2 outTexcoord;

layout (set = 0, binding = 0) uniform A {
	vec2 scale;
	vec2 offset;
} transform_2d;

void main() {
	outTexcoord = inTexcoord;
	gl_Position = vec4(inPosition * transform_2d.scale + transform_2d.offset, 0.0, 1.0);
}
