#include "shader_include.hlsli"

struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXCOORD0;
};

float4 main( VS_OUTPUT Input ) : SV_TARGET
{
	float2 texCoord = Input.texCoord;
	float3 cColor = (float3)g_baseTexture.Sample(g_samPoint, texCoord);
	//cColor *= cColor;
	return float4(pow(saturate(cColor.rgb), 0.8f), 1.0f);
}