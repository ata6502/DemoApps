struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

struct MaterialDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = SpecularPower
};

// Both cbuffers are assigned the same register because they are associated with 
// different shaders.

// Associated with the vertex shader.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix Model;
    matrix View;
    matrix Projection;
};

// Associated with the pixel shader.
cbuffer LightMaterialEyeConstantBuffer : register(b0)
{
    DirectionalLight Light;
    MaterialDesc Material;
    float3 EyePosition;
};
