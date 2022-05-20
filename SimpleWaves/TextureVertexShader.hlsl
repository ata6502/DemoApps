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

    // Transform the input texture coordinates.
    // First, we augment the 2D texture coordinates to a 4D vector to transform
    // the texture using a 4 × 4 matrix.
    // Then, we multiply the 4D vector by a 4 x 4 TextureTransform matrix.
    // After the multiplication, the resulting 4D vector is cast back to 
    // a 2D vector by throwing away the z- and w-components.
    output.Tex = mul(float4(input.Tex, 0.0f, 1.0f), TextureTransform).xy;

    return output;
}