struct DirectionalLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float Pad;
};

struct PointLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;

    float3 Attenuation;
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
    DirectionalLightDesc DirectionalLight;
};

cbuffer ConstantBufferPerFrame : register(b1)
{
    matrix ViewProj;
    PointLightDesc PointLight;
    float3 EyePosition;
    float Pad;
};

cbuffer ConstantBufferPerObject : register(b2)
{
    matrix World;
    MaterialDesc Material;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 posL : POSITION; // a position in local coordinates
    float3 normal : NORMAL;
};

struct PixelShaderInput
{
    float4 posH : SV_POSITION;  // a position in homogenous coordinates
    float3 posW : POSITION; // a position in world space
    float3 normal : NORMAL;
};


