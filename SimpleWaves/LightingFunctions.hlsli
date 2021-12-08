
void ComputeDirectionalLight(Material material, DirectionalLight light, float3 surfacePoint, float3 normal, float3 eyePosition)
{
    float3 L = -light.Direction; // the light vector

    // Apply the formula for diffuse, ambient, and specular components of color.
    float4 A = light.Ambient * material.Ambient;
    float kd = max(dot(L, normal), 0.0f);
    float4 D = light.Diffuse * material.Diffuse;

    float ks = 0.0f;
    // Flatten to avoid dynamic branching.
    [flatten]
    if (dot(L, normal) > 0.0f)
    {
        float3 v = normalize(eyePosition - surfacePoint); // the view vector - the unit vector from the surface point P to the eye position E
        float3 r = reflect(-L, normal); // reflection vector; reflect accepts two args: the incident vector and the normal
        float p = material.Specular.w; // specular power
        ks = pow(max(dot(v, r), 0.0f), p);
    }
    float4 S = light.Specular * float4(material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0

    float4 color = A + kd * D + ks * S;

    return color;
}
