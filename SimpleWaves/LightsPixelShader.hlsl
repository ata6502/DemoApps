#include "LightsShaderInclude.hlsli"

float4 main(PixelShaderInput input) : SV_TARGET
{
    // TODO: Pass black color for now.
    return float4(0.0, 0.0, 0.0, 1.0);
}
