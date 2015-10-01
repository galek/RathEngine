#include "TMshader_include.hlsli"

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float4 vSample = 0.0f;
	float4 vColor = 0.0f;
	float2 vSamplePosition;

	[unroll]
	for (int iSample = 0; iSample < 15; iSample++)
	{
		// Sample from adjacent points
		vSamplePosition = Input.Tex + g_avSampleOffsets[iSample].xy;
		vColor = s0.Sample(g_samPoint, vSamplePosition);

		vSample += g_avSampleWeights[iSample] * vColor;
	}

	return vSample;
}