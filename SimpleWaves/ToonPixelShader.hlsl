#include "LightsShaderInclude.hlsli"
#include "ToonLightSources.hlsli"

float4 main(PixelShaderInput input) : SV_TARGET
{
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
    ComputeToonDirectionalLight(Material, DirectionalLight, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    ComputeToonPointLight(Material, PointLight, input.PosW, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    ComputeToonSpotLight(Material, SpotLight, input.PosW, input.Normal, toEyeW, a, d, s);
    A += a; D += d; S += s;

    float4 color = A + D + S;

    return color;
}

