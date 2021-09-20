#include "ColorShaderInclude.hlsli"

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 Position : POSITION;
    float3 Color : COLOR0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.Position, 1.0f);

    // Transform the vertex position into projected space.
    output.Position = mul(pos, mul(World, ViewProj));

    // Pass the color through without modification.
    output.Color = input.Color;

    return output;
}