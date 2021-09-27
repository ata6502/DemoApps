#include "TextureShaderInclude.hlsli"

float4 main(PixelShaderInput input) : SV_TARGET
{
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

    float4 color = A + kd * D + ks * S;

    return color;
}