#include "ConstantBuffers.hlsli"

struct VertexShaderInput
{
    float3 PosL : POSITION; // vertex position in the local (model) space
};

struct VertexShaderOutput // = PixelShaderInput
{
    float4 PosH : SV_POSITION; // vertex position in the homogenous space
    float3 PosL : POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = float4(input.PosL, 1.0f);

    // Transform the vertex position into projected space.
    // Set z = w so that z/w = 1 (i.e., skydome is always on far plane).
    // Use a DSS state that has its depth function set to LESS_EQUAL.
    output.PosH = mul(mul(mul(pos, WorldSky), View), Projection).xyww; // set z = w
    
    // Use local vertex position as cubemap lookup vector.
    output.PosL = input.PosL;
    
    return output;
}