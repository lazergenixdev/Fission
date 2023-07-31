#version 450 core

layout (location = 0) in vec4 position_uv;
layout (location = 0) out vec2 outTexcoord;

void main() {
    outTexcoord = position_uv.zw;
    gl_Position = vec4(position_uv.xy, 0.0, 1.0);
}
