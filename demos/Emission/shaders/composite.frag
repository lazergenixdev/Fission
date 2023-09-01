#version 450 core

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (set = 0, binding = 0) uniform sampler2D color;

void main() {
	fragColor = vec4(texture(color, uv).rgb, 1.0);
}
