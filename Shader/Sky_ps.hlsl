#include "shader_include.hlsli"

struct PS_INPUT
{
	float4 Position			: SV_POSITION;
	float3 Direction		: TEXCOORD0;
	float4 RayleighColor    : TEXCOORD1;
	float4 MieColor			: TEXCOORD2;
};


[earlydepthstencil]
float4 main(PS_INPUT Input) : SV_TARGET
{
	float cos = dot(g_LightDirection.xyz, Input.Direction) / length(Input.Direction);
	float rayleighPhase = 0.75f * (2.0f + 0.5f * cos*cos);
	float miePhase = 1.5f * ((1.0f - g_g2) / (2.0f + g_g2)) * (1.0f + cos*cos) / pow(abs(1.0f + g_g2 + 2.0f * g_g * cos), 1.5f);
	float4 color = (rayleighPhase * Input.RayleighColor + miePhase * Input.MieColor) * 0.5f;
	//Output.Color.b += (1.f - abs(Input.Direction.y)) * 0.04f;
	//Output.Depth = 1.0f;
	return color;
}