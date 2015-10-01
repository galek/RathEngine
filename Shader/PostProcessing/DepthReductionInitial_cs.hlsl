#include "shader_pp_include.hlsli"

cbuffer ReductionConstants : register(b1)
{
	float4x4 Projection;
	float NearClip;
	float FarClip;
}

Texture2D<float>	g_DepthMap : register(t0);
RWTexture2D<float2> g_OutputMap : register(u0);

groupshared float2 depthSamples[NumThreads];
[numthreads(ReductionTGSize, ReductionTGSize, 1)]
void main(in uint3 GroupID : SV_GroupID,
		  in uint3 GroupThreadID : SV_GroupThreadID,
		  uint ThreadIndex : SV_GroupIndex)
{
	float minDepth = 1.0f;
	float maxDepth = 0.0f;

	uint2 textureSize;
	g_DepthMap.GetDimensions(textureSize.x, textureSize.y);

	uint2 samplePos = GroupID.xy * ReductionTGSize + GroupThreadID.xy;
		samplePos = min(samplePos, textureSize - 1);

	float depthSample = g_DepthMap[samplePos];

	if (depthSample < 1.0f)
	{
		// Convert to linear Z
		depthSample = Projection._43 / (depthSample - Projection._33);
		depthSample = saturate((depthSample - NearClip) / (FarClip - NearClip));
		minDepth = min(minDepth, depthSample);
		maxDepth = max(maxDepth, depthSample);
	}

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