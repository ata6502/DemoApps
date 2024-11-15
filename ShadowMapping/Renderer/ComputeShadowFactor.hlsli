// The following code is based on [Luna]

static const float SHADOW_MAP_SIZE = 2048.0f;
static const float SHADOW_MAP_DX = 1.0f / SHADOW_MAP_SIZE;

// ComputeShadowFactor performs ShadowMap test to determine if a pixel is in shadow. 
float ComputeShadowFactor(SamplerComparisonState comparisonSampler,
                          Texture2D shadowMap,
					      float4 shadowPosH)
{
    shadowPosH.xyz /= shadowPosH.w;
    float depth = shadowPosH.z;

    const float dx = SHADOW_MAP_DX; // texel size

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

	[unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(
            comparisonSampler,
			shadowPosH.xy + offsets[i], 
            depth).r;
    }

    // Average the samples.
    return percentLit /= 9.0f;
}
