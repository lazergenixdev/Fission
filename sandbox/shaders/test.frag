#version 450 core

layout (location = 0) in  vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (push_constant) uniform A {
	vec3 time_min_max;
} info;

#define iTime (info.time_min_max.x)
#define iMin  (info.time_min_max.y)
#define iMax  (info.time_min_max.z)

#define mycos(x) cos(x) * ((iMax-iMin)/2.0) + ((iMax+iMin)/2.0)

vec2 vecFromAngle(float rad) {
    return vec2(cos(rad), sin(rad));
}

float sum(vec2 v) {
    return v.x - v.y;
}

void main() {
    vec2 dir = vecFromAngle(iTime*0.5);

    vec3 col = vec3(mycos(iTime*2.0 + sum(uv.xy*dir)));

    fragColor = vec4(col,1.0);
}
