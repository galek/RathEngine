#include "Skinning.hlsli"

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;
	float4 vEdgeTessellationFactors = g_vTessellationFactor.xxxy;

	// Assign tessellation levels
	Output.EdgeTessFactor[0] = vEdgeTessellationFactors.x;
	Output.EdgeTessFactor[1] = vEdgeTessellationFactors.y;
	Output.EdgeTessFactor[2] = vEdgeTessellationFactors.z;
	Output.InsideTessFactor  = vEdgeTessellationFactors.w;

#if FRUSTUM_CULLING_OPTIMIZATION==1
	// View frustum culling
	bool bViewFrustumCull = ViewFrustumCull(ip[0].vPosition, ip[1].vPosition, ip[2].vPosition, g_vFrustumPlaneEquation, g_vDetailTessellationHeightScale.x);
	if (bViewFrustumCull)
	{
		// Set all tessellation factors to 0 if frustum cull test succeeds
		Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor  = 0.0;
	}
#endif

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
	Output.vPosition = ip[i].vPosition;
	Output.vNormal = ip[i].vNormal;
	Output.texCoord = ip[i].texCoord;

	return Output;
}
