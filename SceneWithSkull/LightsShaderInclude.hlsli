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

cbuffer ConstantBufferNeverChanges : register(b0)
{
    DirectionalLightDesc DirectionalLightArray[3];
};

cbuffer ConstantBufferPerFrame : register(b1)
{
    matrix ViewProj;
    float3 EyePosition;
    float Pad;
};

cbuffer ConstantBufferPerObject : register(b2)
{
    matrix World;
    matrix WorldInvTranspose;
    MaterialDesc Material;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 PosL : POSITION; // a position in local coordinates
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PixelShaderInput
{
    float4 PosH : SV_POSITION;  // a position in homogenous coordinates
    float3 PosW : POSITION; // a position in world space
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
};


