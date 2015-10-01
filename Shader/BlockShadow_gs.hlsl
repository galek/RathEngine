#include "shader_include.hlsli"

struct GS_INPUT
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float3 Texture		: TEXCOORD0;
	float2 TDimension	: TEXCOORD1;
};

struct GS_OUTPUT
{
	float4 Position		: SV_POSITION;
	float3 Texture		: TEXCOORD0;
};

[maxvertexcount(4)]
void main(point GS_INPUT input[1], inout TriangleStream< GS_OUTPUT > stream)
{
	GS_OUTPUT output[4];

	output[0].Position = input[0].Position - float4(input[0].Tangent, 0.0f) - float4(input[0].Binormal, 0.0f);
	output[1].Position = input[0].Position + float4(input[0].Tangent, 0.0f) - float4(input[0].Binormal, 0.0f);
	output[2].Position = input[0].Position - float4(input[0].Tangent, 0.0f) + float4(input[0].Binormal, 0.0f);
	output[3].Position = input[0].Position + float4(input[0].Tangent, 0.0f) + float4(input[0].Binormal, 0.0f);

	output[0].Texture = input[0].Texture;
	output[1].Texture = input[0].Texture + float3(input[0].TDimension.x, 0.0f, 0.0f);
	output[2].Texture = input[0].Texture + float3(0.0f, input[0].TDimension.y, 0.0f);
	output[3].Texture = input[0].Texture + float3(input[0].TDimension, 0.0f);

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output[i].Position = mul(output[i].Position, g_mViewProjection);

		stream.Append(output[i]);
	}

	stream.RestartStrip();
}