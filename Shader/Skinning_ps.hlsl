#include "Skinning.hlsli"

float4 main( DS_OUTPUT Input) : SV_TARGET
{
	float4 cTextureColor = float4(0, 0, 0, 1);
	float4 cDiffuseColor = float4(0, 0, 0, 1);

	// Compute resulting color for the pixel:
	cTextureColor = g_baseTexture.Sample(g_samLinear, Input.texCoord);

	if (cTextureColor.a < 0.2)
		discard;

	// Calc diffuse color    
	cDiffuseColor = Input.vColor * cTextureColor;
	return cDiffuseColor;
}