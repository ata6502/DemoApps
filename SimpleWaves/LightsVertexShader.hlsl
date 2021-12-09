#include "LightsShaderInclude.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.Position, 1.0f);

    // Transform the vertex position into projected space.
    output.Position = mul(pos, mul(World, ViewProj));

    // Transform the normal to world space to have correct lighting.
    // Note that by setting w=0 we don't apply translation to the normals.
    float4 normal = float4(input.Normal, 0.0f);
    normal = mul(normal, World);
    output.Normal = normalize(normal).xyz;

    return output;
}