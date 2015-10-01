#include "shader_lum_include.hlsli"

// Creates the luminance map for the scene
float4 main(in PS_INPUT input) : SV_Target
{
	// Sample the input
	float3 color = g_tex0.Sample(g_samLinear, input.TexCoord).rgb;

	// calculate the luminance using a weighted average
	float luminance = log(max(CalcLuminance(color), 0.00001f));

	return float4(luminance, 1.0f, 1.0f, 1.0f);
}