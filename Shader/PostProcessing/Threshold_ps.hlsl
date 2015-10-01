#include "shader_lum_include.hlsli"

// Uses a lower exposure to produce a value suitable for a bloom pass
float4 main(in PS_INPUT input) : SV_Target
{
    float3 color = 0;

    color = g_tex0.Sample(g_samLinear, input.TexCoord).rgb;

    // Tone map it to threshold
    float avgLuminance = GetAvgLuminance(g_tex1);
    float exposure = 0;
    float pixelLuminance = CalcLuminance(color);
    color = CalcExposedColor(color, avgLuminance, g_BloomThreshold, exposure);

    if(dot(color, 0.333f) <= 0.001f)
        color = 0.0f;

    return float4(color, 1.0f);
}
