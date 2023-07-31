#version 450 core

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (set = 0, binding = 0) uniform sampler2D depth;

// hard-coded :(
const vec2 resolution = vec2(1280.0, 720.0);

float getPixelDepth(int x, int y) {
    return texture(depth, uv + vec2(float(x)/resolution.x, float(y)/resolution.y)).r;
}

void main() {
    float d = getPixelDepth(0, 0);
    float depthDiff = 0.0;
    depthDiff += abs(d - getPixelDepth(1, 0));
    depthDiff += abs(d - getPixelDepth(-1,0));
    depthDiff += abs(d - getPixelDepth(0, 1));
    depthDiff += abs(d - getPixelDepth(0,-1));

    fragColor = vec4(vec3(depthDiff), 1.0);
    //fragColor = vec4(vec3(d), 1.0);
}
