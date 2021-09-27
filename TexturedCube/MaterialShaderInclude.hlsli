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
    float Pad;
};

cbuffer ConstantBufferPerObject : register(b2)
{
    matrix World;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 posL : POSITION; // a position in local coordinates
    float3 normal : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 posH : SV_POSITION;  // a position in homogenous coordinates
    float3 posW : POSITION; // a position in world space
    float3 normal : NORMAL;
};

