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

cbuffer CBufferNeverChanges : register(b0)
{
    DirectionalLightDesc Light;
};

cbuffer CBufferOnResize : register(b1)
{
    matrix Projection;
};

cbuffer CBufferPerFrame : register(b2)
{
    matrix View;
    float3 EyePosition;
    float Pad;
};

cbuffer CBufferPerObject : register(b3)
{
    matrix World;
    matrix WorldInvTranspose;
    MaterialDesc Material;
    matrix TextureTransform;
};

cbuffer CBufferSky : register(b3)
{
    matrix WorldSky;
};
