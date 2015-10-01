#include "shader_lum_include.hlsli"

// Slowly adjusts the scene luminance based on the previous scene luminance
float4 main(in PS_INPUT input) : SV_Target
{
    float lastLum = g_tex0.Load(uint3(0, 0, 0)).x;
    float currentLum = exp(g_tex1.SampleLevel(g_samPoint, float2(0.5f, 0.5f), 10.0f).x);

    // Adapt the luminance using Pattanaik's technique
	float adaptedLum = clamp(lastLum + (currentLum - lastLum) * (1 - exp(-g_TimeDelta * g_Tau)), g_MaxAdept, g_MinAdept);

    return float4(adaptedLum, 1.0f, 1.0f, 1.0f);
}