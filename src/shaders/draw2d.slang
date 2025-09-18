
struct Vertex
{
    float2 position;
    float2 texcoord;
    float4 color;
}

struct VOut
{
    float4 position : SV_Position;
    float2 texcoord;
    float4 color;
}

struct Uniforms
{
	float4x4 transform;
}

[vk::push_constant]
Uniforms u;

[shader("vertex")]
VOut main(Vertex input)
{
    VOut output;
    float4 pos = mul(u.transform, float4(input.position, 0, 1));
    output.position = float4(pos.xy, 0, 1);
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 main(VOut input) : SV_Target
{
    return input.color * texture.Sample(input.texcoord);
}
