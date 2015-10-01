#include "Skinning.hlsli"

/*VS_CONTROL_POINT_OUTPUT*/ DS_OUTPUT main(VS_INPUT input)
{
	//VS_CONTROL_POINT_OUTPUT Output;
	DS_OUTPUT Output;
	SKINNED_OUTPUT Skinned = SkinVert(input);

	// Transform the position from object space to homogeneous projection space
	float4 vPositionWS = mul(Skinned.vPosition, g_mWorld);

	// Transform the normal from object space to world space    
	float3 vNormalWS = normalize(mul(Skinned.vNormal, (float3x3)g_mWorld));

	// Just copy the texture coordinate through
	Output.texCoord = input.inTexCoord;
	//Output.vPosition = vPositionWS.xyz;
	//Output.vNormal = vNormalWS;

	Output.vPosition = mul(vPositionWS, g_mViewProjection);

	Output.vColor = g_MaterialDiffuseColor * g_LightDiffuse * max(0, dot(vNormalWS, g_LightDirection.xyz)) + g_MaterialAmbientColor + g_LightAmbient;
	Output.vColor.a = 1.0f;

	return Output;
}