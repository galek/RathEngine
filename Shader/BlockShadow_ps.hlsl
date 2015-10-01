#include "shader_include.hlsli"

Texture2DArray g_Texture	: register(t0);

struct PS_INPUT
{
	noperspective centroid float4	Position : SV_POSITION;
	float3			Texture	 : TEXCOORD0;
};

float main(PS_INPUT input) : SV_DepthGreaterEqual
{
	float alpha = g_Texture.Sample(g_samPoint, input.Texture).a;
	if (alpha < 0.1f)
		discard;

	return input.Position.z;
}