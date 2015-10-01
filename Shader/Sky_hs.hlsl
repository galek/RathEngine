#include "shader_include.hlsli"

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float3 Position : WORLDPOS;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 Position : WORLDPOS; 
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// View frustum culling
	bool bViewFrustumCull = ViewFrustumCull(ip[0].Position, ip[1].Position, ip[2].Position, g_vCenterFrustumPlaneEquation, 0.0f);

	if (bViewFrustumCull)
	{
		Output.EdgeTessFactor[0] =
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.InsideTessFactor = 0.0f;
	}
	else
	{
		Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.InsideTessFactor = 10.0f;
	}

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.Position = ip[i].Position;

	return Output;
}
