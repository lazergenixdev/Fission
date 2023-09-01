#version 450 core

layout (location = 0) out vec4 fragColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput color;

void main() {
	 fragColor = subpassLoad(color);
}
