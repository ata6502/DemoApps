#include "ConstantBuffers.hlsli"

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex     : TEXCOORD;
};

// Per-pixel color data passed through the pixel shader.
struct VertexShaderOutput // = PixelShaderInput
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = float4(input.PosL, 1.0f);

    // Transform the vertex position into projected space.
    output.PosH = mul(mul(mul(pos, World), View), Projection);

    // Transform the normal to world space. The world inverse-transpose matrix is used 
    // to properly transform normals if there are any non-uniform or shear transformations.
    // Note that by setting w=0 we don't apply translation to the normals.
    float4 normal = float4(input.NormalL, 0.0f);
    normal = mul(normal, WorldInvTranspose);
    output.NormalW = normalize(normal).xyz;

    // Transform the vertex position to world space. We need that for specular light calculations.
    output.PosW = mul(float4(input.PosL, 1.0f), World).xyz;
    
    // Transform the input texture coordinates.
    output.Tex = mul(float4(input.Tex, 0.0f, 1.0f), TextureTransform).xy;

    return output;
}