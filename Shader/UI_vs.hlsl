#include "shader_include.hlsli"

struct VS_INPUT
{
	float3 Position     : POSITION;
	float2 Texture		: TEXCOORD0;
	float4 Color		: COLOR;
};

struct VS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float2 Texture			: TEXCOORD0;
	float4 Color			: COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = float4(input.Position.x * 2.f - 1.f, input.Position.y * 2.f - 1.f, input.Position.z, 1.f);
	output.Texture = input.Texture;
	output.Color = input.Color;

	return output;
}