#include "shader_include.hlsli"

struct PS_INPUT
{
	float4 Position			: SV_POSITION;
	float2 Texture			: TEXCOORD0;
	float4 Color			: COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 FinalColor = g_baseTexture.Sample(g_samAnisotropic, input.Texture) * input.Color;

	return FinalColor;
}