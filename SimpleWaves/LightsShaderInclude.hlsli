cbuffer ConstantBufferPerFrame : register(b0)
{
    matrix ViewProj;
};

cbuffer ConstantBufferPerObject : register(b1)
{
    matrix World;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

