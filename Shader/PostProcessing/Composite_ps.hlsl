#include "shader_lum_include.hlsli"

// Applies exposure and tone mapping to the input, and combines it with the
// results of the bloom pass
float4 main(in PS_INPUT input) : SV_Target
{
    // Tone map the primary input
    float avgLuminance = GetAvgLuminance(g_tex1);
    float3 color = g_tex0.Sample(g_samPoint, input.TexCoord).rgb;

    float exposure = 0;
    color = ToneMap(color, avgLuminance, 0, exposure);
	float lumen = sqrt(dot(color, float3(0.299f, 0.587f, 0.114f)));

    // Sample the bloom
    float3 bloom = g_tex2.Sample(g_samLinear, input.TexCoord).rgb;
    bloom *= g_BloomMagnitude;

    color = color + bloom;

	return float4(color, lumen);
}