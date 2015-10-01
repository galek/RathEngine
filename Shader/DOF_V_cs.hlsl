#define GridSize_ 450
#define ApronSize_ 10
#define MaxSampleRadius_ 10
#define TGSize_ 470

//=================================================================================================
// Structures
//=================================================================================================
struct DOFSample
{
	float3 Color;
	float Depth;
	float Blur;
};

//=================================================================================================
// Resources
//=================================================================================================

// Inputs
Texture2D<float4> ColorTexture : register(t0);
Texture2D<float2> DepthBlurTexture : register(t1);

// Samplers
SamplerState PointSampler : register(s0);

// Outputs
RWTexture2D<float4> OutputTexture : register(u0);

// Shared memory
groupshared DOFSample Samples[TGSize_];

//=================================================================================================
// Performs the vertical DOF pass
//=================================================================================================
[numthreads(1, TGSize_, 1)]
void main(uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID,
	uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
	// These positions are relative to the "grid", AKA the vertical group of pixels
	// that this thread group is writing to
	const int gridStartY = GroupID.y * GridSize_;
	const int gridY = GroupThreadID.y - ApronSize_;

	// These positions are relative to the pixel coordinates
	const int sampleX = GroupID.x;
	const int sampleY = gridStartY + gridY;

	int2 textureSize;
	ColorTexture.GetDimensions(textureSize.x, textureSize.y);

	const int2 samplePos = int2(sampleX, sampleY);

	// Sample the textures
	float2 sampleCoord = saturate((samplePos + 0.5f) / float2(textureSize));
	float3 color = ColorTexture.SampleLevel(PointSampler, sampleCoord, 0.0f).xyz;
	float2 depthBlur = DepthBlurTexture.SampleLevel(PointSampler, sampleCoord, 0.0f).xy;
	float depth = depthBlur.x;
	float blur = depthBlur.y;
	float cocSize = blur * MaxSampleRadius_;

	// Store in shared memory
	Samples[GroupThreadID.y].Color = color;
	Samples[GroupThreadID.y].Depth = depth;
	Samples[GroupThreadID.y].Blur = blur;

	GroupMemoryBarrierWithGroupSync();

	// Don't continue for threads in the apron, and threads outside the render target size
	if (gridY >= 0 && gridY < GridSize_ && sampleY < textureSize.y)
	{
		[branch]
		if (cocSize > 0.0f)
		{
			float3 outputColor = 0.0f;
				float totalContribution = 0.0f;

			// Gather sample taps inside the radius
			for (int y = -MaxSampleRadius_; y <= MaxSampleRadius_; ++y)
			{
				// Grab the sample from shared memory
				int groupTapY = GroupThreadID.y + y;
				DOFSample tap = Samples[groupTapY];

				// Reject the sample if it's outside the CoC radius
				float cocWeight = saturate(cocSize + 1.0f - abs(float(y)));

				// Reject foreground samples, unless they're blurred as well
				float depthWeight = tap.Depth >= depth;
				float blurWeight = tap.Blur;
				float tapWeight = cocWeight * saturate(depthWeight + blurWeight);

				outputColor += tap.Color * tapWeight;
				totalContribution += tapWeight;
			}

			// Write out the result
			OutputTexture[samplePos] = float4(outputColor / totalContribution, 1.0f);
		}
		else
			OutputTexture[samplePos] = float4(color, 1.0f);
	}
}