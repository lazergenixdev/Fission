#version 450 core

layout (location = 0) in vec3 inPositionEmmission;
layout (location = 1) in vec4 inColor;
layout (location = 0) out float outEmmission;

layout (set = 0, binding = 0) uniform A {
	vec2 scale;
	vec2 offset;
} transform_2d;

void main() {
	outEmmission = inPositionEmmission.z;
	gl_Position = vec4(inPositionEmmission.xy * transform_2d.scale + transform_2d.offset, 
	inColor.r // use inColor so that Vulkan does not complain like a baby
	, 1.0);
}
