
struct VS_OUTPUT
{
	float4 Position  : SV_POSITION;
	float2 TexCoord  : TEXCOORD0;
};

Texture2D s0 : register(t0);

SamplerState	g_samLinear		: register(s1);

// Approximates luminance from an RGB value
float CalcLuminance(float3 color)
{
	return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

// Creates the luminance map for the scene
float main(in VS_OUTPUT input) : SV_Target
{
	// Sample the input
	float3 color = s0.Sample(g_samLinear, input.TexCoord).rgb;

	// calculate the luminance using a weighted average
	float luminance = CalcLuminance(color);

	return luminance;
}