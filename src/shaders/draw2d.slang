
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

[shader("vertex")]
VOut vertexMain(Vertex input)
{
    VOut output;
    output.position = float4(input.position.x, -input.position.y, 0.0, 1.0);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return input.color;
}
