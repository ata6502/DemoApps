#include "LightsShaderInclude.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posL, 1.0f);

    // Transform the vertex position into projected space.
    output.posH = mul(pos, mul(World, ViewProj));

    // Transform the normal to world space to have correct lighting.
    // Note that by setting w=0 we don't apply translation to the normals.
    float4 normal = float4(input.normal, 0.0f);
    normal = mul(normal, World);
    output.normal = normalize(normal).xyz;

    // Transform the vertex position to world space. We need that for specular light calculations.
    output.posW = mul(float4(input.posL, 1.0f), World).xyz;

    return output;
}