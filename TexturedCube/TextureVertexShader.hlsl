#include "TextureShaderInclude.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.PosL, 1.0f);

    // Transform the vertex position into projected space.
    output.PosH = mul(pos, mul(World, ViewProj));

    // Transform the normal to world space to have correct lighting.
    // Note that by setting w=0 we don't apply translation to the normals.
    float4 normal = float4(input.Normal, 0.0f);
    normal = mul(normal, World);
    output.Normal = normalize(normal).xyz;

    // Transform the vertex position to world space. We need that for specular light calculations.
    output.PosW = mul(float4(input.PosL, 1.0f), World).xyz;

    // Transform the input texture coordinates:
    // - augment the 2D texture coordinates to a 4D vector
    // - multiply the 4D vector by a 4 x 4 TextureTransform matrix
    // - cast the resulting 4D vector is back to a 2D vector by throwing away the z- and w-components
    output.Tex = mul(float4(input.Tex, 0.0f, 1.0f), TextureTransform).xy;

    return output;
}