#include "shader_include.hlsli"

struct VS_INPUT
{
	float3 Position			: POSITION;
	//float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Diffuse			: COLOR;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
	float4x4 World			: WORLD;
};

struct VS_OUTPUT
{
	float4 Position			: POSITION0;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Texture = input.Texture;
	output.Position = mul(float4(input.Position, 1.0f), input.World);
	output.Tangent = mul(input.Tangent, (float3x3)input.World);
	output.Binormal = mul(input.Binormal, (float3x3)input.World);

	output.TDimension = input.TDimension;

	return output;
}