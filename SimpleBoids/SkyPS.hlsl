#include "ConstantBuffers.hlsli"

struct PixelShaderInput // = VertexShaderOutput
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

// Declare the cubemap.
TextureCube gCubeMap : register(t0);

// Declare a linear sampler.
SamplerState gLinearSampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Sample the sky texture.
    return gCubeMap.Sample(gLinearSampler, input.PosL);
}
