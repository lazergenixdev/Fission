#version 450 core

layout (location = 0) in  float inEmmission;
layout (location = 0) out float fragColor;

//layout (push_constant) uniform A {
//	float emission_scale;
//} i;

void main() {
    fragColor = inEmmission;
}
