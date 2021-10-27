#include "ColorShaderInclude.hlsli"

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