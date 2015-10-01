#include "shader_include.hlsli"

struct DS_OUTPUT
{
	float4 Position		: SV_POSITION;
	float3 Texture		: TEXCOORD0;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 Position		: POSITION;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float3 Texture		: TEXCOORD0;
	float2 TDimension	: TEXCOORD1;
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

	Output.Position = mul(patch[0].Position + float4(patch[0].Tangent, 0.0f) * uv.x + float4(patch[0].Binormal, 0.0f) * uv.y, g_mViewProjection);
	Output.Texture = patch[0].Texture + float3(patch[0].TDimension * uv, 0.0f);

	return Output;
}
