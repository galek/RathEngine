#include "Skinning.hlsli"

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	float3 vWorldPos =  patch[0].vPosition*domain.x + 
						patch[1].vPosition*domain.y + 
						patch[2].vPosition*domain.z;

	float3 vNormal =	patch[0].vNormal*domain.x + 
						patch[1].vNormal*domain.y + 
						patch[2].vNormal*domain.z;
	vNormal	= normalize(vNormal);

	Output.texCoord	=	patch[0].texCoord*domain.x + 
						patch[1].texCoord*domain.y + 
						patch[2].texCoord*domain.z;

	//// Calculate MIP level to fetch normal from
	//float fHeightMapMIPLevel = clamp((distance(vWorldPos, g_vEye) - 100.0f) / 100.0f, 0.0f, 3.0f);

	//// Sample normal and height map
	//float4 vNormalHeight = g_nmhTexture.SampleLevel(g_samLinear, Output.texCoord, fHeightMapMIPLevel);

	//// Displace vertex along normal
	//vWorldPos += vNormal * (g_vDetailTessellationHeightScale.x * (vNormalHeight.w - 1.0));

	// Transform world position with viewprojection matrix
	Output.vPosition = mul(float4(vWorldPos, 1.0), g_mViewProjection);

	Output.vColor = g_MaterialDiffuseColor * g_LightDiffuse * max(0, dot(vNormal, g_LightDirection.xyz)) + g_MaterialAmbientColor + g_LightAmbient;
	Output.vColor.a = 1.0f;

	return Output;
}
