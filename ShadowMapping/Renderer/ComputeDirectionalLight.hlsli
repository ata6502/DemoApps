void ComputeDirectionalLight(
    MaterialDesc material, DirectionalLightDesc light, float3 normal, float3 toEye,
    out float4 ambientColor, out float4 diffuseColor, out float4 specularColor)
{
    // Initialize the outputs.
    ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuseColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Determine the light vector L.
    float3 L = -light.Direction;

    // Calculate the ambient component.
    ambientColor = light.Ambient * material.Ambient;

    // Calculate the diffuse factor.
    float kd = dot(L, normal);

    [flatten]
    if (kd > 0.0f)
    {
        // Calculate the diffuse component.
        diffuseColor = kd * light.Diffuse * material.Diffuse;

        // Calculate the specular factor ks.
        float3 r = reflect(-L, normal); 
        float p = material.Specular.w;
        float ks = pow(max(dot(r, toEye), 0.0f), p);

        // Calculate the specular component.
        specularColor = ks * light.Specular * material.Specular;
    }
}