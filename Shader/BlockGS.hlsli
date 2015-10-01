#include "shader_include.hlsli"

struct GS_INPUT
{
	float4 Position			: ANCHOR;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor    : COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
	float3 LTexture			: TEXCOORD2;
};

struct GS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor    : COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float3 LTexture			: TEXCOORD1;
	float4 WorldPosition	: TEXCOORD2;
#ifdef SHADOWMAPPING
	float4 TextureShadow	: TEXCOORD3;
#endif
};

void GS(point GS_INPUT input[1], inout TriangleStream< GS_OUTPUT > stream)
{
	GS_OUTPUT output[4];

	output[0].Position = -float4(input[0].Tangent, 0.0f) - float4(input[0].Binormal, 0.0f);
	output[1].Position =  float4(input[0].Tangent, 0.0f) - float4(input[0].Binormal, 0.0f);
	output[2].Position = -float4(input[0].Tangent, 0.0f) + float4(input[0].Binormal, 0.0f);
	output[3].Position =  float4(input[0].Tangent, 0.0f) + float4(input[0].Binormal, 0.0f);

	output[0].Texture = input[0].Texture - float3(input[0].TDimension, 0.0f);
	output[1].Texture = input[0].Texture + float3(input[0].TDimension.x, -input[0].TDimension.y, 0.0f);
	output[2].Texture = input[0].Texture + float3(-input[0].TDimension.x, input[0].TDimension.y, 0.0f);
	output[3].Texture = input[0].Texture + float3(input[0].TDimension, 0.0f);

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		output[i].LTexture = (input[0].LTexture + output[i].Position.xyz).xzy / float3(34.f, 34.f, 258.f);
		output[i].Position += input[0].Position;
		output[i].WorldPosition = output[i].Position;
#ifdef SHADOWMAPPING
		output[i].TextureShadow = mul(output[i].Position, g_mShadow);
#endif
		output[i].Position = mul(output[i].Position, g_mViewProjection);
		output[i].Diffuse = input[0].Diffuse;
		output[i].RayleighColor = input[0].RayleighColor;
		output[i].MieColor = input[0].MieColor;

		stream.Append(output[i]);
	}	

	stream.RestartStrip();
}