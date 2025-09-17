// TODO https://www.shadertoy.com/view/3fXfDf

static const int SAMPLE_COUNT = 6;

static const float OFFSETS[6] = float[6](
    -4.378621204796657,
    -2.431625915613778,
    -0.4862426846689485,
    1.4588111840004858,
    3.4048471718931532,
    5
);

static const float WEIGHTS[6] = float[6](
    0.09461172151436463,
    0.20023097066826712,
    0.2760751120037518,
    0.24804559825032563,
    0.14521459357563646,
    0.035822003987654526
);

struct Uniforms {
    float2 direction;
    float2 size;
}

[vk::push_constant]
Uniforms u;

float4 blur(Sampler2D source, float2 pixelCoord)
{
    float4 result = float4(0.0);
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 offset = u.direction * OFFSETS[i] / u.size;
        float weight = WEIGHTS[i];
        result += source.Sample(pixelCoord + offset) * weight;
    }
	return result;
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
	// direction == 0 => copy image with multiplied by size.x
    if (dot(u.direction, u.direction) == 0.0f)
        return texture.Sample(input.texcoord) * u.size.x;
	// direction != 0 => blur image on direction
    return blur(texture, input.texcoord);
}
