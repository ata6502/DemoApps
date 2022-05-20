#include "TextureShaderInclude.hlsli"
#include "LightSources.hlsli"

// Declare a texture a.k.a. a diffuse map.
Texture2D gTexture : register(t0);

// Declare a linear sampler. SamplerState mirrors the interface ID3D11SamplerState.
SamplerState gLinearSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Sample the texture.
    float4 texColor = gTexture.Sample(gLinearSampler, input.Tex);

    // Normalize the input normal vector as interpolation may have unnormalize it.
    input.Normal = normalize(input.Normal);

    // toEyeW is the view vector: a unit vector from the surface point P to the eye position E.
    float3 toEyeW = normalize(EyePosition - input.PosW); 

    // Initialize the scene light components. 
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f); // ambient component
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f); // diffuse component
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f); // specular component

    // Declare light components used to calculate light for each source.
    float4 a, d, s;

    // Sum the light contribution from each light source.
    ComputeDirectionalLight(Material, DirectionalLight, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    ComputePointLight(Material, PointLight, input.PosW, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    ComputeSpotLight(Material, SpotLight, input.PosW, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    // Modulate the texture color with the ambient and diffuse lighting.
    float4 color = texColor * (A + D) + S;

    // Take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}

