#include "ColorShaderInclude.hlsli"

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.Color, 1.0f);
}
