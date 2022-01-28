// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(input.Color, 1.0f);
}
