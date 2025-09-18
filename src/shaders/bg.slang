
[vk::push_constant]
cbuffer a {
    float2 iResolution;
    float iTime;
    float _padding;
}

#if 0
vec3 pal(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d)
{
    return a + b * cos(6.28318 * (c * t + d));
}
vec2 p = fragCoord.xy / iResolution.xy;
p.x += 0.9;
vec3 col = pal(p.x, vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(1.0, 0.7, 0.4), vec3(0.0, 0.15, 0.20));
#endif

float sdPentagram(float2 p, float r)
{
    const float k1x = 0.809016994; // cos(π/ 5) = ¼(√5+1)
    const float k2x = 0.309016994; // sin(π/10) = ¼(√5-1)
    const float k1y = 0.587785252; // sin(π/ 5) = ¼√(10-2√5)
    const float k2y = 0.951056516; // cos(π/10) = ¼√(10+2√5)
    const float k1z = 0.726542528; // tan(π/ 5) = √(5-2√5)
    const float2 v1 = float2( k1x,-k1y);
    const float2 v2 = float2(-k1x,-k1y);
    const float2 v3 = float2( k2x,-k2y);
    
    p.x = abs(p.x);
    p -= 2.0*max(dot(v1,p),0.0)*v1;
    p -= 2.0*max(dot(v2,p),0.0)*v2;
    p.x = abs(p.x);
    p.y -= r;
    return length(p-v3*clamp(dot(p,v3),0.0,k1z*r))
           * sign(p.y*v3.x-p.x*v3.y);
}

float saturate(float x)
{
    return min(max(x, 0.0), 1.0);
}

float circle_mask(float2 center, float radius, float2 pos)
{
    return step(1.0*sdPentagram(2.0*(center - pos), 2.0*radius), 0.0);
}

float radius_from_uv(float2 uv)
{
    float sd = dot(uv, float2(1,-1));
    return saturate(sd*0.6 + 0.5);
}

struct VOut
{
    float4 position : SV_Position;
    float2 texcoord;
}

static const float2[3] full_screen_triangle = float2[3](
    float2(3.0, 1.0),
    float2(-1.0, 1.0),
    float2(-1.0, -3.0)
);

[shader("vertex")]
VOut main(uint vertex_id : SV_VulkanVertexID)
{
    VOut output;
    float2 pos = full_screen_triangle[vertex_id];
    output.position = float4(pos, 0.0, 1.0);
    output.texcoord = (pos + 1.0) * 0.5;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 main(VOut input) : SV_Target
{
    float2 fragCoord = input.position.xy;
    const float2 direction = float2(1.0, -0.8);
    const float max_radius = 0.707106781187; // 1 / sqrt 2
    
    float2 uv = fragCoord/iResolution.xy;
    float2 p = 2.0*(fragCoord/iResolution.y) - 1.0;
    float2 np = fract(p * 4.0 - direction * iTime);

    float value = saturate(0.0
    +   circle_mask(float2(0.0, 0.0), max_radius * radius_from_uv(uv), np)
    +   circle_mask(float2(1.0, 1.0), max_radius * radius_from_uv(uv), np)
    +   circle_mask(float2(0.0, 1.0), max_radius * radius_from_uv(uv), np)
    +   circle_mask(float2(1.0, 0.0), max_radius * radius_from_uv(uv), np)
    );
    
    float3 col1 = pow(float3(0.2, 0.1, 0.2)*0.95, 2.2);
    float3 col2 = pow(float3(0.2, 0.1, 0.2), 2.2);
    float3 col = lerp(col1, col2, value);

    // Output to screen
    return float4(col, 1.0);
}
