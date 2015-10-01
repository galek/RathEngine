#include "TMshader_include.hlsli"

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	//float4 vColor = 0;
	float4 vColor = s0.Sample(g_samPoint, Input.Tex);
	float4 vLum = s1.Sample(g_samPoint, float2(0, 0));
	float3 vBloom = s2.Sample(g_samLinear, Input.Tex).rgb;

	// Tone mapping
	vColor.rgb *= vColor.rgb;// +float3(0.1, 0.1, 0.1);
	vColor.rgb *= MIDDLE_GRAY / (max(vLum.r,0.0f) + 0.01f);
	vColor.rgb *= (1.0f + vColor.rgb / LUM_WHITE);
	vColor.rgb /= (1.0f + vColor.rgb);

	vColor.rgb += 0.6f * vBloom;
	vColor.a = dot(vColor.rgb, float3(0.299, 0.587, 0.114));

	return vColor;
}