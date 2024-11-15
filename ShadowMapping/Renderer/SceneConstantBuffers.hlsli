struct DirectionalLightDesc
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

cbuffer CBufferPerFrame : register(b0)
{
    matrix View;
    matrix Projection;
    DirectionalLightDesc DirectionalLight;
    float3 EyePosition;
    float Pad;
};

cbuffer CBufferPerObject : register(b1)
{
    matrix World;
    matrix WorldInvTranspose;
    MaterialDesc Material;
    matrix TextureTransform;
    matrix ShadowTransform;
};




