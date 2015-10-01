#include "shader_include.hlsli"

Texture2DArray	g_TextureNormal	: register(t0);

struct DS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor    : COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float3 LTexture			: TEXCOORD1;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 Position			: ANCHOR;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 Diffuse			: COLOR0;
	float4 RayleighColor	: COLOR1;
	float4 MieColor			: COLOR2;
	float3 Texture			: TEXCOORD0;
	float2 TDimension		: TEXCOORD1;
	float3 LTexture			: TEXCOORD2;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor[2]		: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 1

[domain("quad")]
DS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT input, float2 domain : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	float2 uv = domain * 2.0f - float2(1.0f, 1.0f);

	Output.Position = float4(patch[0].Tangent * uv.x + patch[0].Binormal * uv.y, 0.0f);
	Output.Texture = float3(domain, patch[0].Texture.z);
	Output.LTexture = (patch[0].LTexture + Output.Position.xyz) / float3(34.0f, 256.0f, 34.0f);
	//float3 vNormal = g_TextureNormal.SampleLevel(g_samBilinear, Output.Texture, 0).rgb;

	//float3x3 fMatrix = { patch[0].Binormal, patch[0].Normal, patch[0].Tangent };

	//Output.Texture = float3(Output.Position.yz * patch[0].Normal.x + Output.Position.xz * patch[0].Normal.y + Output.Position.xy * patch[0].Normal.z, patch[0].Texture.z);

	//Output.LTexture = (input.LTexture + Output.Position.xyz).xzy / float3(34.f, 34.f, 258.f);
	Output.Position += patch[0].Position;
	Output.Position = mul(Output.Position, g_mViewProjection);
	Output.Diffuse = patch[0].Diffuse;
	Output.RayleighColor = patch[0].RayleighColor;
	Output.MieColor = patch[0].MieColor;// .rgb, dot(vNormal, input.LightTS));



	return Output;
}
