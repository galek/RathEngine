
//=================================================================================================
// Constants
//=================================================================================================
cbuffer PPConstants : register(b0)
{
	float BloomThreshold			: packoffset(c0.x);
	float BloomMagnitude			: packoffset(c0.y);
	float BloomBlurSigma			: packoffset(c0.z);
	float Tau						: packoffset(c0.w);
	float TimeDelta					: packoffset(c1.x);
	float KeyValue					: packoffset(c1.y);
	float GatherBlurSize			: packoffset(c1.z);
	float2 ProjectionAB				: packoffset(c2.x);
	float4 DOFDepths				: packoffset(c3.x);
	float MaxBokehSize				: packoffset(c4.x);
	float BokehBrightnessThreshold	: packoffset(c4.y);
	float BokehBlurThreshold		: packoffset(c4.z);
	float BokehFalloff				: packoffset(c4.w);
	float BokehDepthCullThreshold	: packoffset(c5.x);
	float BokehDepthCutoff			: packoffset(c5.y);
};

struct VS_OUTPUT
{
	float4 Position  : SV_POSITION;
	float2 TexCoord  : TEXCOORD0;
};

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);

SamplerState	g_samPoint		: register(s0);
SamplerState	g_samLinear		: register(s1);

// Slowly adjusts the scene luminance based on the previous scene luminance
float main(in VS_OUTPUT input) : SV_Target
{
	float lastLum = exp(s0.Sample(g_samPoint, input.TexCoord).x);
	float currentLum = s1.Sample(g_samPoint, input.TexCoord).x;

	// Adapt the luminance using Pattanaik's technique
	float adaptedLum = lastLum + (currentLum - lastLum) * (1 - exp(-TimeDelta * Tau));

	return log(adaptedLum);
}