#include "TextureShaderInclude.hlsli"

// Declare a texture a.k.a. a diffuse map.
Texture2D gTexture : register(t0);

// Declare a linear sampler.
SamplerState gLinearSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Default to multiplicative identity. This is a default texture 
    // sample in case we wanted to make texturing optional.
    float4 texColor = float4(1, 1, 1, 1);

    // Sample the texture.
    texColor = gTexture.Sample(gLinearSampler, input.tex);

    float3 L = -Light.Direction; // the light vector

    // Apply the formula for diffuse, ambient, and specular components of color.
    float4 A = Light.Ambient * Material.Ambient;
    float kd = max(dot(L, input.normal), 0.0f);
    float4 D = Light.Diffuse * Material.Diffuse;

    float ks = 0.0f;
    // Flatten to avoid dynamic branching.
    [flatten]
    if (dot(L, input.normal) > 0.0f)
    {
        float3 v = normalize(EyePosition - input.posW); // the view vector - the unit vector from the surface point P to the eye position E
        float3 r = reflect(-L, input.normal); // reflection vector; reflect accepts two args: the incident vector and the normal
        float p = Material.Specular.w; // specular power
        ks = pow(max(dot(v, r), 0.0f), p);
    }
    float4 S = Light.Specular * float4(Material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0

    float4 ambient = A;
    float4 diffuse = kd * D;
    float4 spec = ks* S;

    // Modulate with late add.
    float4 color = texColor * (ambient + diffuse) + spec;

    // Common to take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}