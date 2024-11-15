#include "ShadowConstantBuffers.hlsli"

struct VertexShaderInput
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex     : TEXCOORD;
};

struct VertexShaderOutput // = PixelShaderInput
{
    float4 PosH : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = float4(input.PosL, 1.0f);

    // Transform the vertex position into the light's projected space.
    output.PosH = mul(mul(mul(pos, World), View), Projection);
    
    return output;
}