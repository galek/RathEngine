#include "shader_include.hlsli"

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float4 Position		: POSITION;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float3 Texture		: TEXCOORD0;
	float2 TDimension	: TEXCOORD1;
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
#define BACKFACE_EPSILON 0.75

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float tessellationFactor = 1.0f;// +pow(GetDistanceAdaptiveScaleFactor(g_vEye.xyz, ip[0].Position.xyz, 10.f, 100.f), 4.0f) * 8.0f;

	if (ViewFrustumCull(ip[0].Position.xyz, g_vFrustumPlaneEquation, BACKFACE_EPSILON))
	{
		Output.EdgeTessFactor[0] =
			Output.EdgeTessFactor[1] =
			Output.EdgeTessFactor[2] =
			Output.EdgeTessFactor[3] =
			Output.InsideTessFactor[0] =
			Output.InsideTessFactor[1] = 0;
	}
	else
	{
		Output.EdgeTessFactor[0] =
			Output.EdgeTessFactor[1] =
			Output.EdgeTessFactor[2] =
			Output.EdgeTessFactor[3] =
			Output.InsideTessFactor[0] =
			Output.InsideTessFactor[1] = tessellationFactor;
	}

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.Position = ip[i].Position;
	Output.Tangent = ip[i].Tangent;
	Output.Binormal = ip[i].Binormal;
	Output.Texture = ip[i].Texture;
	Output.TDimension = ip[i].TDimension;

	return Output;
}
