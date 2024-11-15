#include "SceneConstantBuffers.hlsli"
#include "ComputeDirectionalLight.hlsli"
#include "ComputeShadowFactor.hlsli"

struct PixelShaderInput // = VertexShaderOutput
{
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float2 Tex        : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

// Declare a texture a.k.a. a diffuse map.
Texture2D gTexture : register(t0);

// Declare the shadow map (a.k.a. depth map) texture.
Texture2D gShadowMapTexture : register(t1);

// Declare a linear sampler.
SamplerState gLinearSampler : register(s0);

// Declare a comparison sampler.
SamplerComparisonState gComparisonSampler : register(s1);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Initialize the scene light components. 
    float4 A = float4(0, 0, 0, 0); // ambient component
    float4 D = float4(0, 0, 0, 0); // diffuse component
    float4 S = float4(0, 0, 0, 0); // specular component

    // Normalize the input normal vector as interpolation may have unnormalized it.
    input.NormalW = normalize(input.NormalW);

    float4 texColor = gTexture.Sample(gLinearSampler, input.Tex);

    // toEyeW is the view vector: a unit vector from the surface point P to the eye position E.
    float3 toEyeW = normalize(EyePosition - input.PosW);

    // Calculate the shadow factor.
    float shadow = ComputeShadowFactor(gComparisonSampler, gShadowMapTexture, input.ShadowPosH);
    
    ComputeDirectionalLight(Material, DirectionalLight, input.NormalW, toEyeW, A, D, S);

    // Multiply the shadow against the diffuse and specular lighting terms.
    float4 color = texColor * (A + shadow * D) + shadow * S;

    // Take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}
