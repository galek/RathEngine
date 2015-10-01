#include "TMshader_include.hlsli"

float4 main(VS_OUTPUT Input) : SV_TARGET
{
	float4 vColor = 0.0f;
	float  fAvg = 0.0f;

	[unroll]
	for (int y = 0; y < 2; y++)
	{
		[unroll]
		for (int x = 0; x < 2; x++)
		{
			// Compute the sum of color values
			vColor = s0.Sample(g_samPoint, Input.Tex, int2(x, y));
			fAvg += dot(vColor, LUM_VECTOR);
		}
	}

	fAvg /= 4;

	return float4(fAvg, fAvg, fAvg, 1.0f);
}