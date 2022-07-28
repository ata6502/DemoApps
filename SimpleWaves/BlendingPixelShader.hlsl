#include "TextureShaderInclude.hlsli"
#include "LightSources.hlsli"

#ifndef ENABLE_FOG
#define ENABLE_FOG 1
#endif

#ifndef ENABLE_CLIPPING 
#define ENABLE_CLIPPING 1
#endif

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

    // toEyeW is the view vector: a vector from the surface point P to the eye position E.
    float3 toEyeW = EyePosition - input.PosW; 

    // Cache the length of the unnormalized view vector.
    float distToEye = length(toEyeW);

    // Normalize the view vector.
    toEyeW /= distToEye;

#if ENABLE_CLIPPING==1
    // Use the intrinsic HLSL clip function to completely reject a source pixel. We can call the clip
    // function only in a pixel shader. It is useful for rendering completely opaque or completely 
    // transparent pixels.

    // We grab the alpha component of the texture (texColor.a) and subtract from it a small value 
    // close to 0 such as 0.1. If the result is less than 0 it means that the pixel is completely 
    // or almost transparent. The clip function discards the pixel if its argument is less than 0.

    // Note that we do this test as soon as possible so that we can potentially exit the shader early, 
    // thereby skipping the rest of the shader code. The HLSL clip instruction corresponds to the discard 
    // instruction in assembly.
    clip(texColor.a - 0.1f);
#endif

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

    // Blend the fog color and the lit color.
#if ENABLE_FOG==1
    float fogLerp = saturate((distToEye - FogStart) / FogRange);

    // Blend the fog color and the lit color.
    color = lerp(color, FogColor, fogLerp); // the same as: (1 - fogLerp)*color + fogLerp*FogColor; this is the transparency blending equation
#endif

    // Take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}

