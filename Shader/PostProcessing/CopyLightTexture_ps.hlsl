#include "../shader_include.hlsli"

struct VS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXCOORD0;
};

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float3 cColor = (float3)g_baseTexture.Sample(g_samLinear, Input.texCoord);
	return float4(cColor.rgb, 1.0f);
}