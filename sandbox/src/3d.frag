#version 450 core

layout (location = 0) in float inDistance;
layout (location = 0) out vec4 fragColor;

layout (push_constant) uniform A {
    vec4 position;
    vec4 color;
    float time;
} info;

void main() {
    // transform distance to be between 0.0-1.0
    float normal_distance = 1.0;
    if (inDistance != 0.0) normal_distance = 1.0 / (1.0 - inDistance);
    
    // source color
    vec3 c = info.color.rgb;

    // calculate value
    float value = exp((mod(-inDistance - info.time, 32.0) - 32.0) / 4.0);

    // modify color with value
    c = clamp(c + vec3(value), vec3(0.0), vec3(1.0));

    // adjust color to fade into distance
    fragColor = vec4(c * normal_distance, 1.0);
}
