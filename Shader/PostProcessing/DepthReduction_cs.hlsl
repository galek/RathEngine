#include "shader_pp_include.hlsli"

Texture2D<float2>	g_ReductionMap : register(t0);
RWTexture2D<float2> g_OutputMap : register(u0);

groupshared float2 depthSamples[NumThreads];

[numthreads(ReductionTGSize, ReductionTGSize, 1)]
void main(in uint3 GroupID : SV_GroupID, 
		  in uint3 GroupThreadID : SV_GroupThreadID,
		  in uint ThreadIndex : SV_GroupIndex)
{
	uint2 textureSize;
	g_ReductionMap.GetDimensions(textureSize.x, textureSize.y);

	uint2 samplePos = GroupID.xy * ReductionTGSize + GroupThreadID.xy;
		samplePos = min(samplePos, textureSize - 1);

	float minDepth = g_ReductionMap[samplePos].x;
	float maxDepth = g_ReductionMap[samplePos].y;

	if (minDepth == 0.0f)
		minDepth = 1.0f;

	// Store in shared memory
	depthSamples[ThreadIndex] = float2(minDepth, maxDepth);
	GroupMemoryBarrierWithGroupSync();

	// Reduce
	[unroll]
	for (uint s = NumThreads / 2; s > 0; s >>= 1)
	{
		if (ThreadIndex < s)
		{
			depthSamples[ThreadIndex].x = min(depthSamples[ThreadIndex].x, depthSamples[ThreadIndex + s].x);
			depthSamples[ThreadIndex].y = max(depthSamples[ThreadIndex].y, depthSamples[ThreadIndex + s].y);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	if (ThreadIndex == 0)
	{
		minDepth = depthSamples[0].x;
		maxDepth = depthSamples[0].y;
		g_OutputMap[GroupID.xy] = float2(minDepth, maxDepth);
	}
}