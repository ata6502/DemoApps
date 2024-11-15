#include "SceneConstantBuffers.hlsli"

struct VertexShaderInput
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex     : TEXCOORD;
};

struct VertexShaderOutput // = PixelShaderInput
{
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float2 Tex        : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = float4(input.PosL, 1.0f);

    // Transform the vertex position into projected space.
    output.PosH = mul(mul(mul(pos, World), View), Projection);

    // Transform the normal to world space. 
    float4 normal = float4(input.NormalL, 0.0f);
    normal = mul(normal, WorldInvTranspose);
    output.NormalW = normalize(normal).xyz;

    // Transform the vertex position to world space.
    output.PosW = mul(float4(input.PosL, 1.0f), World).xyz;

    // Transform the input texture coordinates.
    output.Tex = mul(float4(input.Tex, 0.0f, 1.0f), TextureTransform).xy;

    // Generate projective texture coords to project shadow map onto scene.
    output.ShadowPosH = mul(float4(input.PosL, 1.0f), ShadowTransform);
    
    return output;
}