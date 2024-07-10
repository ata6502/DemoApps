#include "ConstantBuffers.hlsli"
#include "ComputeDirectionalLight.hlsli"

struct PixelShaderInput
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
};

// Declare a texture a.k.a. a diffuse map.
Texture2D gTexture : register(t0);

// Declare a linear sampler.
SamplerState gLinearSampler : register(s0);

// Calculate pixel color using normals, light, and material.
float4 main(PixelShaderInput input) : SV_TARGET
{
    // Initialize the scene light components. 
    float4 A = float4(0, 0, 0, 0); // ambient component
    float4 D = float4(0, 0, 0, 0); // diffuse component
    float4 S = float4(0, 0, 0, 0); // specular component

    // Normalize the input normal vector as interpolation may have unnormalized it.
    input.NormalW = normalize(input.NormalW);

    // Sample the texture.
    float4 texColor = gTexture.Sample(gLinearSampler, input.Tex);

    // toEyeW is the view vector: a unit vector from the surface point P to the eye position E.
    float3 toEyeW = normalize(EyePosition - input.PosW);

    ComputeDirectionalLight(Material, Light, input.NormalW, toEyeW, A, D, S);

    // Modulate with late add.
    float4 color = texColor * (A + D) + S;

    // Take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}
