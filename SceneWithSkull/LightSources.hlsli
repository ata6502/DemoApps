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

        // Calculate the specular component.
        specularColor = specFactor * light.Specular * float4(material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0
    }
}

