#include "ColorShaderInclude.hlsli"

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.Position, 1.0f);

    pos.xy += 0.5f * sin(pos.x) * sin(3.0f * Time.x);
    pos.z *= 0.6f + 0.4f * sin(2.0f * Time.x);

    // Transform the vertex position into projected space.
    output.Position = mul(pos, mul(World, ViewProj));

    // Pass the color through without modification.
    output.Color = input.Color;

    return output;
}