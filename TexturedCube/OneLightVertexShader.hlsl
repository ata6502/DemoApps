#include "OneLightShaderInclude.hlsli"

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 posL : POSITION; // renamed 'pos' to 'posL' to underline that this arg is in local coordinates
    float3 normal : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 posH : SV_POSITION;  // renamed 'pos' to 'posH' to underline that this arg is in homogenous coordinates
    float3 posW : POSITION; // a position in world space
    float3 normal : NORMAL;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posL, 1.0f);

    // Transform the vertex position into projected space.
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    output.posH = pos;

    // Transform the normal to world space to have correct lighting.
    // Note that by setting w=0 we don't apply translation to the normals.
    float4 normal = float4(input.normal, 0.0f);
    normal = mul(normal, Model);
    output.normal = normalize(normal).xyz;

    // Transform the vertex position to world space. We need that for specular light calculations.
    output.posW = mul(float4(input.posL, 1.0f), Model).xyz;

    return output;
}