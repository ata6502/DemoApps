
// ComputeDirectionalLight outputs the lit color of a point given a material, directional light source, 
// surface normal, and the unit vector from the surface point being lit to the eye.
// It applies the formula 7.3 for diffuse, ambient, and specular components of color.
void ComputeDirectionalLight(float3 normal, float3 toEye,
    out float4 ambientColor, out float4 diffuseColor, out float4 specularColor)
{
    // Initialize outputs.
    ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuseColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Determine the light vector L. It is opposite to the direction of light rays.
    float3 L = -Light.Direction;

    // Calculate the ambient component.
    ambientColor = Light.Ambient * Material.Ambient;

    // Calculate the diffuse factor. If it is greater than zero it means that the surface 
    // is facing the light source i.e., it is lit.
    float diffuseFactor = max(dot(L, normal), 0.0f);

    // Flatten to avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        // Calculate the diffuse component.
        diffuseColor = diffuseFactor * Light.Diffuse * Material.Diffuse;

        // Determine the reflection vector r. The reflect function accepts two args: the incident vector and the normal.
        float3 r = reflect(-L, normal); 

        // Grab the specular power.
        float p = Material.Specular.w; 

        // Calculate the specular factor. toEye is the view vector.
        float specFactor = pow(max(dot(r, toEye), 0.0f), p);

        // Calculate the diffuse component.
        specularColor = specFactor * Light.Specular * float4(Material.Specular.xyz, 1.0f); // replace SpecularPower component (stored in w) with 1.0
    }
}



