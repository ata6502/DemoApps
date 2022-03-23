/*
    [Luna] Ex.3 p.278 (Section 7.16)
    One characteristic of toon lighting is the abrupt transition from one color shade to the next (in contrast with a smooth transition).
    This can be implemented by computing kd and ks in the usual way, but then transforming them by discrete functions before using them 
    in the pixel shader. Modify the lighting demo of this chapter to use this sort of toon shading.
*/
void ComputeToonDirectionalLight(
    MaterialDesc material, DirectionalLightDesc light, float3 normal, float3 toEye,
    out float4 ambientColor, out float4 diffuseColor, out float4 specularColor)
{
    // Initialize outputs.
    ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuseColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Determine the light vector L. It is opposite to the direction of light rays.
    float3 L = -light.Direction;

    // Calculate the ambient component.
    ambientColor = light.Ambient * material.Ambient;

    // Calculate the diffuse factor. If it is greater than zero it means that the surface 
    // is facing the light source i.e., it is lit.
    float diffuseFactor = max(dot(L, normal), 0.0f);

    // Apply toon effect to the diffuse factor kd.
    [flatten]
    if (diffuseFactor <= 0.0f)
        diffuseFactor = 0.4f;
    else if (diffuseFactor <= 0.5f)
        diffuseFactor = 0.6f;
    else if (diffuseFactor <= 1.0f)
        diffuseFactor = 1.0f;

    // Calculate the diffuse component.
    diffuseColor = diffuseFactor * light.Diffuse * material.Diffuse;

    // Determine the reflection vector r. The reflect function accepts two args: the incident vector and the normal.
    float3 r = reflect(-L, normal); 

    // Grab the specular power.
    float p = material.Specular.w; 

    // Calculate the specular factor. toEye is the view vector.
    float specFactor = pow(max(dot(r, toEye), 0.0f), p);

    // Apply toon effect to the specular factor ks.
    [flatten]
    if (specFactor <= 0.1f)
        specFactor = 0.0f;
    else if (specFactor <= 0.8f)
        specFactor = 0.5f;
    else if (specFactor <= 1.0f)
        specFactor = 0.8f;

    // Calculate the specular component.
    specularColor = specFactor * light.Specular * float4(material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0
}

void ComputeToonPointLight(
    MaterialDesc material, PointLightDesc light, float3 position, float3 normal, float3 toEye,
    out float4 ambientColor, out float4 diffuseColor, out float4 specularColor)
{
    // Initialize outputs.
    ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuseColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The light vector points from the surface to the light source.
    float3 L = light.Position - position; // 'position' is a point on the surface

    // The distance from the surface to the light source.
    float d = length(L);

    // Range test.
    if (d > light.Range)
        return;

    // Normalize the light vector.
    L /= d;

    // Calculate the ambient component.
    ambientColor = material.Ambient * light.Ambient;

    // Calculate the diffuse factor.
    float diffuseFactor = dot(L, normal);

    // Apply toon effect to the diffuse factor kd.
    [flatten]
    if (diffuseFactor <= 0.0f)
        diffuseFactor = 0.4f;
    else if (diffuseFactor <= 0.5f)
        diffuseFactor = 0.6f;
    else if (diffuseFactor <= 1.0f)
        diffuseFactor = 1.0f;

    diffuseColor = diffuseFactor * material.Diffuse * light.Diffuse;

    float3 r = reflect(-L, normal); // reflection vector
    float p = material.Specular.w; // specular power
    float specFactor = pow(max(dot(r, toEye), 0.0f), p);

    // Apply toon effect to the specular factor ks.
    [flatten]
    if (specFactor <= 0.1f)
        specFactor = 0.0f;
    else if (specFactor <= 0.8f)
        specFactor = 0.5f;
    else if (specFactor <= 1.0f)
        specFactor = 0.8f;

    specularColor = specFactor * material.Specular * light.Specular;

    // Attenuate
    // The use of dot: light.Attenuation is float3 and we perform dot product of two float3 values to obtain a0+a1*d+a2*d^2
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, d, d * d)); 

    diffuseColor *= att;
    specularColor *= att;
}

void ComputeToonSpotLight(
    MaterialDesc material, SpotLightDesc light, float3 position, float3 normal, float3 toEye,
    out float4 ambientColor, out float4 diffuseColor, out float4 specularColor)
{
    // Initialize outputs.
    ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuseColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // The light vector points from the surface to the light source.
    float3 L = light.Position - position; // 'position' is a point on the surface

        // The distance from surface to light source.
    float d = length(L);

    // Range test.
    if (d > light.Range)
        return;

    // Normalize the light vector.
    L /= d;

    // Calculate the ambient component.
    ambientColor = material.Ambient * light.Ambient;

    // Calculate the diffuse factor.
    float diffuseFactor = dot(L, normal);

    // Apply toon effect to the diffuse factor kd.
    [flatten]
    if (diffuseFactor <= 0.f)
        diffuseFactor = 0.4f;
    else if (diffuseFactor <= 0.5f)
        diffuseFactor = 0.6f;
    else if (diffuseFactor <= 1.0f)
        diffuseFactor = 1.0f;

    diffuseColor = diffuseFactor * material.Diffuse * light.Diffuse;

    float3 r = reflect(-L, normal); // reflection vector
    float p = material.Specular.w; // specular power
    float specFactor = pow(max(dot(r, toEye), 0.0f), p);

    // Apply toon effect to the specular factor ks.
    [flatten]
    if (specFactor <= 0.1f)
        specFactor = 0.0f;
    else if (specFactor <= 0.8f)
        specFactor = 0.5f;
    else if (specFactor <= 1.0f)
        specFactor = 0.8f;

    specularColor = specFactor * material.Specular * light.Specular;

    // Compute the intensity falloff: kspot(f) = max(cos(f),0)^s = max(-L dot d,0)^s where d is the light direction.
    float spot = pow(max(dot(-L, light.Direction), 0.0f), light.Spot);

    // Scale by spotlight factor and attenuate.
    float att = spot / dot(light.Attenuation, float3(1.0f, d, d * d)); // att = kspot / (a0 + a1*d + a2*d^2)

    // Apply the spot light equation [Luna] 7.5
    ambientColor *= spot;  // ambient  = kspot * ambient
    diffuseColor *= att;   // diffuse  = kspot * diffuseFactor * diffuse / (a0 + a1*d + a2*d^2)
    specularColor *= att;  // specular = kspot * specFactor * specular / (a0 + a1*d + a2*d^2)
}

