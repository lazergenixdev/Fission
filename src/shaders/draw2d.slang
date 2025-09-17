
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

[vk::push_constant]
float4 offset_scale;

[shader("vertex")]
VOut main(Vertex input)
{
    VOut output;
    output.position = float4(input.position.xy * offset_scale.zw + offset_scale.xy, 0, 1);
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 main(VOut input) : SV_Target
{
    return input.color * texture.Sample(input.texcoord);
    //return input.color * float4(input.texcoord, 1, 1);
}
