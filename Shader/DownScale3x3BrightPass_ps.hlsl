#include "TMshader_include.hlsli"

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float3 vColor = 0.0f;
	float4 vLum = s1.Sample(g_samPoint, float2(0, 0));
	float  fLum = vLum.r;

	vColor = s0.Sample(g_samPoint, Input.Tex).rgb;

	// Bright pass and tone mapping
	vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);
	vColor *= MIDDLE_GRAY / (max(fLum,0.0f) + 0.01f);
	vColor *= (1.0f + vColor.rgb / LUM_WHITE);
	vColor /= (1.0f + vColor.rgb);

	return float4(vColor, 1.0f);
}