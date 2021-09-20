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

cbuffer ConstantBufferNeverChanges : register(b0)
{
    DirectionalLight Light;
    MaterialDesc Material;
};

cbuffer ConstantBufferPerFrame : register(b1)
{
    matrix ViewProj;
    float3 EyePosition;
};

cbuffer ConstantBufferPerObject : register(b2)
{
    matrix World;
};
