
void ComputeDirectionalLight(
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

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        // Calculate the diffuse component.
        diffuseColor = diffuseFactor * light.Diffuse * material.Diffuse;

        // Determine the reflection vector r. The reflect function accepts two args: the incident vector and the normal.
        float3 r = reflect(-L, normal); 

        // Grab the specular power.
        float p = material.Specular.w; 

        // Calculate the specular factor. toEye is the view vector.
        float specFactor = pow(max(dot(r, toEye), 0.0f), p);

        // Calculate the diffuse component.
        specularColor = specFactor * light.Specular * float4(material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0
    }
}

void ComputePointLight(
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

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 r = reflect(-L, normal); // reflection vector
        float p = material.Specular.w; // specular power
        float specFactor = pow(max(dot(r, toEye), 0.0f), p);

        diffuseColor = diffuseFactor * material.Diffuse * light.Diffuse;
        specularColor = specFactor * material.Specular * light.Specular;
    }

    // Attenuate
    // The use of dot: light.Attenuation is float3 and we perform dot product of two float3 values to obtain a0+a1*d+a2*d^2
    float att = 1.0f / dot(light.Attenuation, float3(1.0f, d, d * d)); 

    diffuseColor *= att;
    specularColor *= att;
}

void ComputeSpotLight(
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

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 r = reflect(-L, normal); // reflection vector
        float p = material.Specular.w; // specular power
        float specFactor = pow(max(dot(r, toEye), 0.0f), p);

        diffuseColor = diffuseFactor * material.Diffuse * light.Diffuse;
        specularColor = specFactor * material.Specular * light.Specular;
    }

    // Compute the intensity falloff: kspot(f) = max(cos(f),0)^s = max(-L dot d,0)^s where d is the light direction.
    // We also indirectly control the spotlight cone half-angle by altering the exponent s i.e., light.Spot
    float spot = pow(max(dot(-L, light.Direction), 0.0f), light.Spot);

    // Scale by spotlight factor and attenuate.
    float att = spot / dot(light.Attenuation, float3(1.0f, d, d * d)); // att = kspot / (a0 + a1*d + a2*d^2)

    // Apply the spot light equation [Luna] 7.5
    ambientColor *= spot;  // ambient  = kspot * ambient
    diffuseColor *= att;   // diffuse  = kspot * diffuseFactor * diffuse / (a0 + a1*d + a2*d^2)
    specularColor *= att;  // specular = kspot * specFactor * specular / (a0 + a1*d + a2*d^2)
}

