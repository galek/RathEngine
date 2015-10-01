#include "TMshader_include.hlsli"

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float fAvg = 0.0f;
	float4 vColor;

	[unroll]
	for (int y = 0; y <= 2; y++)
	{
		[unroll]
		for (int x = 0; x <= 2; x++)
		{
			// Compute the sum of color values
			vColor = s0.Sample(g_samPoint, Input.Tex, int2(x, y));
			fAvg += vColor.r;
		}
	}

	// Divide the sum to complete the average
	fAvg /= 9;

	return float4(fAvg, fAvg, fAvg, 1.0f);
}