#include "TextureShaderInclude.hlsli"

// This code is based on the book "Introduction to 3D Game Programming with DirectX 11" by Frank Luna
 
// Two directional lights
#define LIGHT_COUNT 2

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
    texColor = gTexture.Sample(gLinearSampler, input.Tex);

    // Start with a sum of zero. 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Sum the light contribution from each light source.  
    [unroll]
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        float3 L = -LightArray[i].Direction; // the light vector

        // Apply the formula for ambient, diffuse, and specular components of color.
        float4 A = LightArray[i].Ambient * Material.Ambient;
        float kd = max(dot(L, input.Normal), 0.0f);
        float4 D = LightArray[i].Diffuse * Material.Diffuse;

        float ks = 0.0f;
        // Flatten to avoid dynamic branching.
        [flatten]
        if (dot(L, input.Normal) > 0.0f)
        {
            float3 v = normalize(EyePosition - input.PosW); // the view vector - the unit vector from the surface point P to the eye position E
            float3 r = reflect(-L, input.Normal); // reflection vector; reflect() accepts two args: the incident vector and the normal
            float p = Material.Specular.w; // specular power
            ks = pow(max(dot(v, r), 0.0f), p);
        }
        float4 S = LightArray[i].Specular * float4(Material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0

        ambient += A;
        diffuse += kd * D;
        spec += ks * S;
    }

    // Modulate with late add.
    float4 color = texColor * (ambient + diffuse) + spec;

    // Common to take alpha from diffuse material and texture.
    color.a = Material.Diffuse.a * texColor.a;

    return color;
}