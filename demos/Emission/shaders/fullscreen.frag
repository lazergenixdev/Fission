#version 450 core

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput color;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput emmission;

void main() {
	 fragColor = subpassLoad(color).rgba * subpassLoad(emmission).r;
}
